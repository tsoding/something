SDL_Texture *render_text_as_texture(SDL_Renderer *renderer,
                                    TTF_Font *font,
                                    const char *text,
                                    SDL_Color color)
{
    SDL_Surface *surface = stec(TTF_RenderText_Blended(font, text, color));
    SDL_Texture *texture = stec(SDL_CreateTextureFromSurface(renderer, surface));
    SDL_FreeSurface(surface);
    return texture;
}

void render_texture(SDL_Renderer *renderer, SDL_Texture *texture, Vec2i p)
{
    int w, h;
    sec(SDL_QueryTexture(texture, NULL, NULL, &w, &h));
    SDL_Rect srcrect = {0, 0, w, h};
    SDL_Rect dstrect = {p.x, p.y, w, h};
    sec(SDL_RenderCopy(renderer, texture, &srcrect, &dstrect));
}

// TODO(#25): Turn displayf into println style
void displayf(SDL_Renderer *renderer, TTF_Font *font,
              SDL_Color color, Vec2i p,
              const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char text[256];
    vsnprintf(text, sizeof(text), format, args);

    SDL_Texture *texture =
        render_text_as_texture(renderer, font, text, color);
    render_texture(renderer, texture, p);
    SDL_DestroyTexture(texture);

    va_end(args);
}

enum class Debug_Draw_State {
    Idle = 0,
    Create,
    Delete
};

struct Game_State
{
    Vec2i gravity;
    bool quit;
    Vec2i collision_probe;
    Vec2i mouse_position;
    Debug_Draw_State state;

    Sprite ground_grass_texture;
    Sprite ground_texture;

    TTF_Font *debug_font;

    int tracking_projectile_index;
};

const int ENEMY_ENTITY_INDEX_OFFSET = 1;
const int ENEMY_COUNT = 6;
const int PLAYER_ENTITY_INDEX = 0;

void render_debug_overlay(Game_State game_state, SDL_Renderer *renderer,
                          int fps)
{
    sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));

    const int COLLISION_PROBE_SIZE = 10;
    const SDL_Rect collision_probe_rect = {
        game_state.collision_probe.x - COLLISION_PROBE_SIZE,
        game_state.collision_probe.y - COLLISION_PROBE_SIZE,
        COLLISION_PROBE_SIZE * 2,
        COLLISION_PROBE_SIZE * 2
    };
    sec(SDL_RenderFillRect(renderer, &collision_probe_rect));

    sec(SDL_RenderDrawRect(renderer, &LEVEL_BOUNDARY));

    const int PADDING = 10;
    displayf(renderer, game_state.debug_font,
             {255, 0, 0, 255}, vec2(PADDING, PADDING),
             "FPS: %d", fps);
    displayf(renderer, game_state.debug_font,
             {255, 0, 0, 255}, vec2(PADDING, 50 + PADDING),
             "Mouse Position: (%d, %d)",
             game_state.mouse_position.x,
             game_state.mouse_position.y);
    displayf(renderer, game_state.debug_font,
             {255, 0, 0, 255}, vec2(PADDING, 2 * 50 + PADDING),
             "Collision Probe: (%d, %d)",
             game_state.collision_probe.x,
             game_state.collision_probe.y);
    displayf(renderer, game_state.debug_font,
             {255, 0, 0, 255}, vec2(PADDING, 3 * 50 + PADDING),
             "Projectiles: %d",
             count_alive_projectiles());

    if (game_state.tracking_projectile_index >= 0) {
        auto projectile = projectiles[game_state.tracking_projectile_index];
        const int SECOND_COLUMN_OFFSET = 500;
        displayf(renderer, game_state.debug_font,
                 {255, 255, 0, 255}, vec2(PADDING + SECOND_COLUMN_OFFSET, PADDING),
                 "State: %s", projectile_state_as_cstr(projectile.state));
        displayf(renderer, game_state.debug_font,
                 {255, 255, 0, 255}, vec2(PADDING + SECOND_COLUMN_OFFSET, 50 + PADDING),
                 "Position: (%d, %d)",
                 projectile.pos.x, projectile.pos.y);
        displayf(renderer, game_state.debug_font,
                 {255, 255, 0, 255}, vec2(PADDING + SECOND_COLUMN_OFFSET, 2 * 50 + PADDING),
                 "Velocity: (%d, %d)",
                 projectile.vel.x, projectile.vel.y);
    }

    for (size_t i = 0; i < entities_count; ++i) {
        if (entities[i].state == Entity_State::Ded) continue;

        sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
        auto dstrect = entity_dstrect(entities[i]);
        sec(SDL_RenderDrawRect(renderer, &dstrect));

        sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
        auto hitbox = entity_hitbox(entities[i]);
        sec(SDL_RenderDrawRect(renderer, &hitbox));
    }

    if (game_state.tracking_projectile_index >= 0) {
        auto hitbox = hitbox_of_projectile(game_state.tracking_projectile_index);
        sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
        sec(SDL_RenderDrawRect(renderer, &hitbox));
    }

    int index = projectile_at_position(game_state.mouse_position);
    if (index >= 0) {
        auto hitbox = hitbox_of_projectile(index);
        sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
        sec(SDL_RenderDrawRect(renderer, &hitbox));
        return;
    }

    sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
    const SDL_Rect tile_rect = {
        game_state.mouse_position.x / TILE_SIZE * TILE_SIZE,
        game_state.mouse_position.y / TILE_SIZE * TILE_SIZE,
        TILE_SIZE, TILE_SIZE
    };
    sec(SDL_RenderDrawRect(renderer, &tile_rect));
}

void render_game_state(const Game_State game_state,
                  SDL_Renderer *renderer)
{
    sec(SDL_SetRenderDrawColor(renderer, 18, 8, 8, 255));
    sec(SDL_RenderClear(renderer));

    render_level(renderer,
                 game_state.ground_grass_texture,
                 game_state.ground_texture);
    render_entities(renderer);
    render_projectiles(renderer);
}

void update_game_state(Game_State game_state, uint32_t dt)
{
    for (int i = 0; i < ENEMY_COUNT; ++i) {
        entity_shoot(&entities[ENEMY_ENTITY_INDEX_OFFSET + i]);
    }

    update_entities(game_state.gravity, dt);
    update_projectiles(dt);
}

const uint32_t STEP_DEBUG_FPS = 60;

int main(void)
{
    sec(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window =
        sec(SDL_CreateWindow(
                "Something",
                0, 0, 800, 600,
                SDL_WINDOW_RESIZABLE));

    SDL_Renderer *renderer =
        sec(SDL_CreateRenderer(
                window, -1,
                SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

    // TODO(#8): replace fantasy_tiles.png with our own assets
    SDL_Texture *tileset_texture = load_texture_from_png_file(
        renderer,
        "assets/sprites/fantasy_tiles.png");

    // TODO(#9): baking assets into executable

    load_spritesheets(renderer);

    auto plasma_pop_animat = load_animat_file("./assets/animats/plasma_pop.txt");
    auto plasma_bolt_animat = load_animat_file("./assets/animats/plasma_bolt.txt");
    auto walking = load_animat_file("./assets/animats/walking.txt");
    auto idle = load_animat_file("./assets/animats/idle.txt");

    init_projectiles(plasma_bolt_animat, plasma_pop_animat);

    const int PLAYER_TEXBOX_SIZE = 64;
    const int PLAYER_HITBOX_SIZE = PLAYER_TEXBOX_SIZE - 20;
    const SDL_Rect texbox = {
        - (PLAYER_TEXBOX_SIZE / 2), - (PLAYER_TEXBOX_SIZE / 2),
        PLAYER_TEXBOX_SIZE, PLAYER_TEXBOX_SIZE
    };
    const SDL_Rect hitbox = {
        - (PLAYER_HITBOX_SIZE / 2), - (PLAYER_HITBOX_SIZE / 2),
        PLAYER_HITBOX_SIZE, PLAYER_HITBOX_SIZE
    };

    entities[PLAYER_ENTITY_INDEX].state = Entity_State::Alive;
    entities[PLAYER_ENTITY_INDEX].texbox = texbox;
    entities[PLAYER_ENTITY_INDEX].hitbox = hitbox;
    entities[PLAYER_ENTITY_INDEX].walking = walking;
    entities[PLAYER_ENTITY_INDEX].idle = idle;
    entities[PLAYER_ENTITY_INDEX].current = &entities[PLAYER_ENTITY_INDEX].idle;

    for (int i = 0; i < ENEMY_COUNT; ++i) {
        entities[ENEMY_ENTITY_INDEX_OFFSET + i].state = Entity_State::Alive;
        entities[ENEMY_ENTITY_INDEX_OFFSET + i].texbox = texbox;
        entities[ENEMY_ENTITY_INDEX_OFFSET + i].hitbox = hitbox;
        entities[ENEMY_ENTITY_INDEX_OFFSET + i].walking = walking;
        entities[ENEMY_ENTITY_INDEX_OFFSET + i].idle = idle;
        entities[ENEMY_ENTITY_INDEX_OFFSET + i].current = &entities[ENEMY_ENTITY_INDEX_OFFSET + i].idle;
        static_assert(LEVEL_WIDTH >= 2);
        entities[ENEMY_ENTITY_INDEX_OFFSET + i].pos = vec2(LEVEL_WIDTH - 2 - i, 0) * TILE_SIZE;
        if (i % 2) {
            entities[ENEMY_ENTITY_INDEX_OFFSET + i].dir = Entity_Dir::Left;
        } else {
            entities[ENEMY_ENTITY_INDEX_OFFSET + i].dir = Entity_Dir::Right;
        }
    }

    stec(TTF_Init());
    const int DEBUG_FONT_SIZE = 32;

    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

    Game_State game_state = {};
    game_state.gravity = {0, 1};
    game_state.debug_font =
        stec(TTF_OpenFont("./assets/fonts/UbuntuMono-R.ttf", DEBUG_FONT_SIZE));
    game_state.ground_grass_texture = {
        {120, 128, 16, 16},
        tileset_texture
    };
    game_state.ground_texture = {
        {120, 128 + 16, 16, 16},
        tileset_texture
    };
    game_state.tracking_projectile_index = -1;

    bool debug = false;
    bool step_debug = false;
    Uint32 prev_dt = 0;

    while (!game_state.quit) {
        const Uint32 begin = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                game_state.quit = true;
            } break;

            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    entities[PLAYER_ENTITY_INDEX].vel.y = -20;
                } break;

                case SDLK_q: {
                    debug = !debug;
                } break;

                case SDLK_z: {
                    step_debug = !step_debug;
                } break;

                case SDLK_x: {
                    if (step_debug) {
                        update_game_state(game_state, 1000 / STEP_DEBUG_FPS);
                    }
                } break;

                case SDLK_e: {
                    entity_shoot(&entities[PLAYER_ENTITY_INDEX]);
                } break;

                case SDLK_r: {
                    entities[PLAYER_ENTITY_INDEX].pos = vec2(0, 0);
                    entities[PLAYER_ENTITY_INDEX].vel.y = 0;
                    for (int i = 0; i < ENEMY_COUNT; ++i) {
                        entities[ENEMY_ENTITY_INDEX_OFFSET + i].pos.y = 0;
                        entities[ENEMY_ENTITY_INDEX_OFFSET + i].vel.y = 0;
                    }
                } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                game_state.mouse_position = {event.motion.x, event.motion.y};
                game_state.collision_probe = game_state.mouse_position;
                resolve_point_collision(&game_state.collision_probe);

                Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
                switch (game_state.state) {
                case Debug_Draw_State::Create: {
                    if (is_tile_inbounds(tile)) level[tile.y][tile.x] = Tile::Wall;
                } break;

                case Debug_Draw_State::Delete: {
                    if (is_tile_inbounds(tile)) level[tile.y][tile.x] = Tile::Empty;
                } break;

                default: {}
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                if (debug) {
                    game_state.tracking_projectile_index =
                        projectile_at_position(vec2(event.button.x, event.button.y));
                    if (game_state.tracking_projectile_index < 0) {
                        Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
                        if (is_tile_inbounds(tile)) {
                            if (level[tile.y][tile.x] == Tile::Empty) {
                                game_state.state = Debug_Draw_State::Create;
                                level[tile.y][tile.x] = Tile::Wall;
                            } else {
                                game_state.state = Debug_Draw_State::Delete;
                                level[tile.y][tile.x] = Tile::Empty;
                            }
                        }
                    }
                }
            } break;

            case SDL_MOUSEBUTTONUP: {
                game_state.state = Debug_Draw_State::Idle;
            } break;
            }
        }

        const int PLAYER_SPEED = 4;
        if (keyboard[SDL_SCANCODE_D]) {
            entity_move(&entities[PLAYER_ENTITY_INDEX], PLAYER_SPEED);
        } else if (keyboard[SDL_SCANCODE_A]) {
            entity_move(&entities[PLAYER_ENTITY_INDEX], -PLAYER_SPEED);
        } else {
            entity_stop(&entities[PLAYER_ENTITY_INDEX]);
        }

        render_game_state(game_state, renderer);
        if (debug) {
            render_debug_overlay(game_state, renderer,
                                 step_debug ? STEP_DEBUG_FPS : 1000 / prev_dt);
        }
        SDL_RenderPresent(renderer);

        if (!step_debug) {
            const Uint32 dt = SDL_GetTicks() - begin;
            update_game_state(game_state, dt);
            prev_dt = dt;
        }
    }
    SDL_Quit();

    dump_level(stdout);

    return 0;
}
