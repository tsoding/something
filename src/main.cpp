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
    Idle,
    Create,
    Delete
};

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
        "assets/fantasy_tiles.png");

    Sprite ground_grass_texture = {
        {120, 128, 16, 16},
        tileset_texture
    };

    Sprite ground_texture = {
        {120, 128 + 16, 16, 16},
        tileset_texture
    };

    Animat plasma_pop_animat = load_spritesheet_animat(
        renderer, 4, 70,
        "./assets/Destroy1-sheet.png");
    Animat plasma_bolt_animat = load_spritesheet_animat(
        renderer, 5, 70,
        "./assets/spark1-sheet.png");
    init_projectiles(plasma_bolt_animat, plasma_pop_animat);

    // TODO(#9): baking assets into executable
    SDL_Texture *walking_texture = load_texture_from_png_file(
        renderer,
        "./assets/walking-12px-zoom.png");

    const int walking_frame_count = 4;
    const int walking_frame_size = 48;
    Sprite walking_frames[walking_frame_count];

    for (int i = 0; i < walking_frame_count; ++i) {
        walking_frames[i].srcrect = {
            i * walking_frame_size,
            0,
            walking_frame_size,
            walking_frame_size
        };
        walking_frames[i].texture = walking_texture;
    }

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

    Animat walking = {};
    walking.frames = walking_frames;
    walking.frame_count = 4;
    walking.frame_duration = 100;

    Animat idle = {};
    idle.frames = walking_frames + 2;
    idle.frame_count = 1;
    idle.frame_duration = 200;

    Entity player = {};
    player.texbox = texbox;
    player.hitbox = hitbox;
    player.walking = walking;
    player.idle = idle;
    player.current = &player.idle;

    Entity supposed_enemy = {};
    supposed_enemy.texbox = texbox;
    supposed_enemy.hitbox = hitbox;
    supposed_enemy.walking = walking;
    supposed_enemy.idle = idle;
    supposed_enemy.current = &supposed_enemy.idle;
    static_assert(LEVEL_WIDTH >= 2);
    supposed_enemy.pos = vec2(LEVEL_WIDTH - 2, 0) * TILE_SIZE;
    supposed_enemy.dir = Entity_Dir::Left;

    stec(TTF_Init());
    const int DEBUG_FONT_SIZE = 32;
    TTF_Font *debug_font = stec(TTF_OpenFont("assets/UbuntuMono-R.ttf", DEBUG_FONT_SIZE));

    Vec2i gravity = {0, 1};
    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

    bool quit = false;
    bool debug = false;
    const int COLLISION_PROBE_SIZE = 10;
    SDL_Rect collision_probe = {};
    Vec2i mouse_position = {};
    SDL_Rect tile_rect = {};
    Debug_Draw_State state = Debug_Draw_State::Idle;

    Uint32 fps = 0;
    while (!quit) {
        const Uint32 begin = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                quit = true;
            } break;

            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    player.vel.y = -20;
                } break;

                case SDLK_q: {
                    debug = !debug;
                } break;

                case SDLK_e: {
                    entity_shoot(&player);
                } break;

                case SDLK_r: {
                    player.pos = vec2(0, 0);
                    player.vel.y = 0;
                } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                Vec2i p = {event.motion.x, event.motion.y};
                resolve_point_collision(&p);

                collision_probe = {
                    p.x - COLLISION_PROBE_SIZE, p.y - COLLISION_PROBE_SIZE,
                    COLLISION_PROBE_SIZE * 2, COLLISION_PROBE_SIZE * 2
                };

                tile_rect = {
                    event.motion.x / TILE_SIZE * TILE_SIZE,
                    event.motion.y / TILE_SIZE * TILE_SIZE,
                    TILE_SIZE, TILE_SIZE
                };

                mouse_position = {event.motion.x, event.motion.y};

                Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
                switch (state) {
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
                    Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
                    if (is_tile_inbounds(tile)) {
                        if (level[tile.y][tile.x] == Tile::Empty) {
                            state = Debug_Draw_State::Create;
                            level[tile.y][tile.x] = Tile::Wall;
                        } else {
                            state = Debug_Draw_State::Delete;
                            level[tile.y][tile.x] = Tile::Empty;
                        }
                    }
                }
            } break;

            case SDL_MOUSEBUTTONUP: {
                state = Debug_Draw_State::Idle;
            } break;
            }
        }

        // entity_move(&supposed_enemy, -1);
        entity_shoot(&supposed_enemy);

        const int PLAYER_SPEED = 4;
        if (keyboard[SDL_SCANCODE_D]) {
            entity_move(&player, PLAYER_SPEED);
        } else if (keyboard[SDL_SCANCODE_A]) {
            entity_move(&player, -PLAYER_SPEED);
        } else {
            entity_stop(&player);
        }

        sec(SDL_SetRenderDrawColor(renderer, 18, 8, 8, 255));
        sec(SDL_RenderClear(renderer));

        render_level(renderer, ground_grass_texture, ground_texture);
        render_entity(renderer, player);
        render_entity(renderer, supposed_enemy);
        render_projectiles(renderer);

        if (debug) {
            sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));

            auto dstrect = entity_dstrect(player);
            sec(SDL_RenderDrawRect(renderer, &dstrect));
            sec(SDL_RenderFillRect(renderer, &collision_probe));
            sec(SDL_RenderDrawRect(renderer, &tile_rect));
            sec(SDL_RenderDrawRect(renderer, &level_boundary));

            const Uint32 t = SDL_GetTicks() - begin;
            const Uint32 fps_snapshot = t ? 1000 / t : 0;
            fps = (fps + fps_snapshot) / 2;

            const int PADDING = 10;
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, PADDING),
                     "FPS: %d", fps);
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, 50 + PADDING),
                     "Mouse Position: (%d, %d)",
                     mouse_position.x, mouse_position.y);
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, 2 * 50 + PADDING),
                     "Collision Probe: (%d, %d)",
                     collision_probe.x, collision_probe.y);
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, 3 * 50 + PADDING),
                     "Projectiles: %d",
                     count_alive_projectiles());


            sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
            auto hitbox = entity_hitbox(player);
            sec(SDL_RenderDrawRect(renderer, &hitbox));
        }

        SDL_RenderPresent(renderer);

        const Uint32 dt = SDL_GetTicks() - begin;

        update_entity(&player, gravity, dt);
        update_entity(&supposed_enemy, gravity, dt);
        update_projectiles(dt);
    }
    SDL_Quit();

    dump_level();

    return 0;
}
