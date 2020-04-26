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

void render_texture(SDL_Renderer *renderer, SDL_Texture *texture, Vec2f p)
{
    int w, h;
    sec(SDL_QueryTexture(texture, NULL, NULL, &w, &h));
    SDL_Rect srcrect = {0, 0, w, h};
    SDL_Rect dstrect = {(int) floorf(p.x), (int) floorf(p.y), w, h};
    sec(SDL_RenderCopy(renderer, texture, &srcrect, &dstrect));
}

// TODO(#25): Turn displayf into println style
void displayf(SDL_Renderer *renderer, TTF_Font *font,
              SDL_Color color, Vec2f p,
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
    Vec2f gravity;
    bool quit;
    bool debug;
    Vec2f collision_probe;
    Vec2f debug_mouse_position;
    Debug_Draw_State state;
    Camera camera;

    Sprite ground_grass_texture;
    Sprite ground_texture;

    TTF_Font *debug_font;

    Maybe<Projectile_Index> tracking_projectile;
};

const size_t ENEMY_ENTITY_INDEX_OFFSET = 1;
const size_t PLAYER_ENTITY_INDEX = 0;

void render_debug_overlay(Game_State game_state, SDL_Renderer *renderer)
{
    sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));

    const float COLLISION_PROBE_SIZE = 10.0f;
    const auto collision_probe_rect = rect(
        game_state.collision_probe - COLLISION_PROBE_SIZE - game_state.camera.pos,
        COLLISION_PROBE_SIZE * 2, COLLISION_PROBE_SIZE * 2);
    {
        auto rect = rectf_for_sdl(collision_probe_rect);
        sec(SDL_RenderFillRect(renderer, &rect));
    }

    auto index = room_index_at(game_state.debug_mouse_position);
    auto room_boundary_screen =
        (ROOM_BOUNDARY + vec2((float) index.unwrap * ROOM_BOUNDARY.w, 1.0f)) - game_state.camera.pos;
    {
        auto rect = rectf_for_sdl(room_boundary_screen);
        sec(SDL_RenderDrawRect(renderer, &rect));
    }

    const float PADDING = 10.0f;
    // TODO(#38): FPS display is broken
    const SDL_Color GENERAL_DEBUG_COLOR = {255, 0, 0, 255};
    displayf(renderer, game_state.debug_font,
             GENERAL_DEBUG_COLOR, vec2(PADDING, PADDING),
             "FPS: %d", 60);
    displayf(renderer, game_state.debug_font,
             GENERAL_DEBUG_COLOR, vec2(PADDING, 50 + PADDING),
             "Mouse Position: (%.4f, %.4f)",
             game_state.debug_mouse_position.x,
             game_state.debug_mouse_position.y);
    displayf(renderer, game_state.debug_font,
             GENERAL_DEBUG_COLOR, vec2(PADDING, 2 * 50 + PADDING),
             "Collision Probe: (%.4f, %.4f)",
             game_state.collision_probe.x,
             game_state.collision_probe.y);
    displayf(renderer, game_state.debug_font,
             GENERAL_DEBUG_COLOR, vec2(PADDING, 3 * 50 + PADDING),
             "Projectiles: %d",
             count_alive_projectiles());
    displayf(renderer, game_state.debug_font,
             GENERAL_DEBUG_COLOR, vec2(PADDING, 4 * 50 + PADDING),
             "Player position: (%.4f, %.4f)",
             entities[PLAYER_ENTITY_INDEX].pos.x,
             entities[PLAYER_ENTITY_INDEX].pos.y);
    displayf(renderer, game_state.debug_font,
             GENERAL_DEBUG_COLOR, vec2(PADDING, 5 * 50 + PADDING),
             "Player velocity: (%.4f, %.4f)",
             entities[PLAYER_ENTITY_INDEX].vel.x,
             entities[PLAYER_ENTITY_INDEX].vel.y);


    if (game_state.tracking_projectile.has_value) {
        auto projectile = projectiles[game_state.tracking_projectile.unwrap.unwrap];
        const float SECOND_COLUMN_OFFSET = 700.0f;
        const SDL_Color TRACKING_DEBUG_COLOR = {255, 255, 0, 255};
        displayf(renderer, game_state.debug_font,
                 TRACKING_DEBUG_COLOR, vec2(PADDING + SECOND_COLUMN_OFFSET, PADDING),
                 "State: %s", projectile_state_as_cstr(projectile.state));
        displayf(renderer, game_state.debug_font,
                 TRACKING_DEBUG_COLOR, vec2(PADDING + SECOND_COLUMN_OFFSET, 50 + PADDING),
                 "Position: (%.4f, %.4f)",
                 projectile.pos.x, projectile.pos.y);
        displayf(renderer, game_state.debug_font,
                 TRACKING_DEBUG_COLOR, vec2(PADDING + SECOND_COLUMN_OFFSET, 2 * 50 + PADDING),
                 "Velocity: (%.4f, %.4f)",
                 projectile.vel.x, projectile.vel.y);
        displayf(renderer, game_state.debug_font,
                 TRACKING_DEBUG_COLOR, vec2(PADDING + SECOND_COLUMN_OFFSET, 3 * 50 + PADDING),
                 "Shooter Index: %d",
                 projectile.shooter.unwrap);
    }

    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        if (entities[i].state == Entity_State::Ded) continue;

        sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
        auto dstrect = rectf_for_sdl(entity_texbox_world(entities[i]) - game_state.camera.pos);
        sec(SDL_RenderDrawRect(renderer, &dstrect));

        sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
        auto hitbox = rectf_for_sdl(entity_hitbox_world(entities[i]) - game_state.camera.pos);
        sec(SDL_RenderDrawRect(renderer, &hitbox));
    }

    if (game_state.tracking_projectile.has_value) {
        sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
        auto hitbox = rectf_for_sdl(
            hitbox_of_projectile(game_state.tracking_projectile.unwrap) - game_state.camera.pos);
        sec(SDL_RenderDrawRect(renderer, &hitbox));
    }

    auto projectile_index = projectile_at_position(game_state.debug_mouse_position);
    if (projectile_index.has_value) {
        sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
        auto hitbox = rectf_for_sdl(
            hitbox_of_projectile(projectile_index.unwrap) - game_state.camera.pos);
        sec(SDL_RenderDrawRect(renderer, &hitbox));
        return;
    }

    sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
    const Rectf tile_rect = {
        floorf(game_state.debug_mouse_position.x / TILE_SIZE) * TILE_SIZE - game_state.camera.pos.x,
        floorf(game_state.debug_mouse_position.y / TILE_SIZE) * TILE_SIZE - game_state.camera.pos.y,
        TILE_SIZE,
        TILE_SIZE
    };
    {
        auto rect = rectf_for_sdl(tile_rect);
        sec(SDL_RenderDrawRect(renderer, &rect));
    }
}

void render_game_state(const Game_State game_state,
                       SDL_Renderer *renderer)
{
    auto index = room_index_at(entities[PLAYER_ENTITY_INDEX].pos);

    const int NEIGHBOR_ROOM_DIM_ALPHA = 200;

    if (index.unwrap > 0) {
        room_row[index.unwrap - 1].render(
            renderer,
            game_state.camera,
            game_state.ground_grass_texture,
            game_state.ground_texture,
            {0, 0, 0, NEIGHBOR_ROOM_DIM_ALPHA});
    }

    room_row[index.unwrap].render(
        renderer,
        game_state.camera,
        game_state.ground_grass_texture,
        game_state.ground_texture);

    if (index.unwrap + 1 < (int) ROOM_ROW_COUNT) {
        room_row[index.unwrap + 1].render(
            renderer,
            game_state.camera,
            game_state.ground_grass_texture,
            game_state.ground_texture,
            {0, 0, 0, NEIGHBOR_ROOM_DIM_ALPHA});
    }

    render_entities(renderer, game_state.camera);
    render_projectiles(renderer, game_state.camera);
}

void update_game_state(Game_State game_state, float dt)
{
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    entity_point_gun_at(
        {PLAYER_ENTITY_INDEX},
        vec2((float) mouse_x, (float) mouse_y) + game_state.camera.pos);

    if (!game_state.debug) {
        for (size_t i = 0; i < ROOM_ROW_COUNT - 1; ++i) {
            size_t player_index = room_index_at(entities[PLAYER_ENTITY_INDEX].pos).unwrap;
            size_t enemy_index = room_index_at(entities[ENEMY_ENTITY_INDEX_OFFSET + i].pos).unwrap;
            if (player_index == enemy_index) {
                entity_point_gun_at(
                    {ENEMY_ENTITY_INDEX_OFFSET + i},
                    entities[PLAYER_ENTITY_INDEX].pos);
                entity_shoot({ENEMY_ENTITY_INDEX_OFFSET + i});
            }
        }
    }

    update_entities(game_state.gravity, dt);
    update_projectiles(dt);

    for (size_t projectile_index = 0;
         projectile_index < projectiles_count;
         ++projectile_index)
    {
        auto projectile = projectiles + projectile_index;
        if (projectile->state != Projectile_State::Active) continue;

        for (size_t entity_index = 0;
             entity_index < ENTITIES_COUNT;
             ++entity_index)
        {
            auto entity = entities + entity_index;

            if (entity->state != Entity_State::Alive) continue;
            if (entity_index == projectile->shooter.unwrap) continue;

            if (rect_contains_vec2(entity_hitbox_world(*entity), projectile->pos)) {
                projectile->state = Projectile_State::Poof;
                projectile->poof_animat.frame_current = 0;
                kill_entity({entity_index});
            }
        }
    }
}

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int SIMULATION_FPS = 60;
const float SIMULATION_DELTA_TIME = 1.0f / SIMULATION_FPS;

void reset_entities(Frame_Animat walking, Frame_Animat idle,
                    Sample_S16 jump_sample1, Sample_S16 jump_sample2)
{
    inplace_spawn_entity(
        {PLAYER_ENTITY_INDEX},
        walking, idle,
        jump_sample1, jump_sample2,
        vec2(ROOM_BOUNDARY.w * 0.5f, ROOM_BOUNDARY.h * 0.5f));

    for (size_t i = 0; i < ROOM_ROW_COUNT - 1; ++i) {
        inplace_spawn_entity({ENEMY_ENTITY_INDEX_OFFSET + i},
                             walking, idle,
                             jump_sample1, jump_sample2,
                             vec2(ROOM_BOUNDARY.w * 0.5f, ROOM_BOUNDARY.h * 0.5f) + room_row[i + 1].position);
    }
}

template <typename T>
void print1(FILE *stream, Vec2<T> v)
{
    print(stream, '(', v.x, ',', v.y, ')');
}

char *file_path_of_room(char *buffer, size_t buffer_size, Room_Index index)
{
    snprintf(buffer, buffer_size, "assets/rooms/room-%lu.bin", index.unwrap);
    return buffer;
}

int main(void)
{
    sec(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));

    Sample_S16 jump_sample1 = load_wav_as_sample_s16("./assets/sounds/jumppp11-48000-mono.wav");
    Sample_S16 jump_sample2 = load_wav_as_sample_s16("./assets/sounds/jumppp22-48000-mono.wav");
    Sample_S16 shoot_sample = load_wav_as_sample_s16("./assets/sounds/enemy_shoot-48000-decay.wav");

    Sample_Mixer mixer = {};
    mixer.volume = 0.2f;

    SDL_AudioSpec want = {};
    want.freq = SOMETHING_SOUND_FREQ;
    want.format = SOMETHING_SOUND_FORMAT;
    want.channels = SOMETHING_SOUND_CHANNELS;
    want.samples = SOMETHING_SOUND_SAMPLES;
    want.callback = sample_mixer_audio_callback;
    want.userdata = &mixer;

    SDL_AudioSpec have = {};
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(
        NULL,
        0,
        &want,
        &have,
        SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    // defer(SDL_CloseAudioDevice(dev));
    if (dev == 0) {
        println(stderr, "SDL pooped itself: Failed to open audio: ", SDL_GetError());
        abort();
    }

    if (have.format != want.format) {
        println(stderr, "[WARN] We didn't get expected audio format.");
        abort();
    }
    SDL_PauseAudioDevice(dev, 0);

    SDL_Window *window =
        sec(SDL_CreateWindow(
                "Something",
                0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
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

    stec(TTF_Init());
    const int DEBUG_FONT_SIZE = 32;

    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

    Game_State game_state = {};
    game_state.gravity = {0.0, 2500.0f};
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
    game_state.tracking_projectile = {};

    char room_file_path[256];
    for (size_t room_index = 0; room_index < ROOM_ROW_COUNT; ++room_index) {
        room_row[room_index].position = {(float) room_index * ROOM_BOUNDARY.w, 0};
        room_row[room_index].load_file(
            file_path_of_room(
                room_file_path,
                sizeof(room_file_path),
                {room_index}));
    }

    reset_entities(walking, idle, jump_sample1, jump_sample2);

    SDL_SetWindowGrab(window, game_state.debug ? SDL_FALSE : SDL_TRUE);

    bool step_debug = false;

    sec(SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND));

    Uint32 prev_ticks = SDL_GetTicks();
    float lag_sec = 0;
    while (!game_state.quit) {
        Uint32 curr_ticks = SDL_GetTicks();
        float elapsed_sec = (float) (curr_ticks - prev_ticks) / 1000.0f;
        prev_ticks = curr_ticks;
        lag_sec += elapsed_sec;

        int window_w = 0, window_h = 0;
        SDL_GetWindowSize(window, &window_w, &window_h);

        //// HANDLE INPUT //////////////////////////////
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                game_state.quit = true;
            } break;

            case SDL_KEYDOWN: {
                if (isdigit(event.key.keysym.sym)) {
                    if (game_state.debug) {
                        int room_index = event.key.keysym.sym - '0' - 1;
                        if (0 <= room_index && (size_t) room_index < ROOM_ROW_COUNT) {
                            auto room_center =
                                vec2(ROOM_BOUNDARY.w * 0.5f, ROOM_BOUNDARY.h * 0.5f) +
                                room_row[room_index].position;
                            entities[PLAYER_ENTITY_INDEX].pos = room_center;
                        }
                    }
                } else {
                    switch (event.key.keysym.sym) {
                    case SDLK_SPACE: {
                        if (!event.key.repeat) {
                            entity_jump({PLAYER_ENTITY_INDEX}, game_state.gravity, &mixer);
                        }
                    } break;

                    case SDLK_q: {
                        game_state.debug = !game_state.debug;
                        SDL_SetWindowGrab(window, game_state.debug ? SDL_FALSE : SDL_TRUE);
                    } break;

                    case SDLK_z: {
                        step_debug = !step_debug;
                    } break;

                    case SDLK_x: {
                        if (step_debug) {
                            update_game_state(game_state, SIMULATION_DELTA_TIME);
                        }
                    } break;

                    case SDLK_e: {
                        auto room_index = room_index_at(entities[PLAYER_ENTITY_INDEX].pos);
                        room_row[room_index.unwrap].dump_file(
                            file_path_of_room(
                                room_file_path,
                                sizeof(room_file_path),
                                room_index));
                        fprintf(stderr, "Saved room %lu to `%s`\n",
                                room_index.unwrap, room_file_path);
                    } break;

                    case SDLK_i: {
                        auto room_index = room_index_at(entities[PLAYER_ENTITY_INDEX].pos);
                        room_row[room_index.unwrap].load_file(
                            file_path_of_room(
                                room_file_path,
                                sizeof(room_file_path),
                                room_index));
                        fprintf(stderr, "Load room %lu from `%s`\n",
                                room_index.unwrap, room_file_path);
                    } break;

                    case SDLK_r: {
                        reset_entities(walking, idle, jump_sample1, jump_sample2);
                    } break;
                    }
                }
            } break;

            case SDL_KEYUP: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    if (!event.key.repeat) {
                        entity_jump({PLAYER_ENTITY_INDEX}, game_state.gravity, &mixer);
                    }
                } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                game_state.debug_mouse_position =
                    vec_cast<float>(vec2(event.motion.x, event.motion.y)) + game_state.camera.pos;
                game_state.collision_probe = game_state.debug_mouse_position;

                auto index = room_index_at(game_state.collision_probe);
                room_row[index.unwrap].resolve_point_collision(&game_state.collision_probe);

                Vec2i tile = vec_cast<int>((game_state.debug_mouse_position - room_row[index.unwrap].position) / TILE_SIZE);
                switch (game_state.state) {
                case Debug_Draw_State::Create: {
                    if (room_row[index.unwrap].is_tile_inbounds(tile))
                        room_row[index.unwrap].tiles[tile.y][tile.x] = Tile::Wall;
                } break;

                case Debug_Draw_State::Delete: {
                    if (room_row[index.unwrap].is_tile_inbounds(tile))
                        room_row[index.unwrap].tiles[tile.y][tile.x] = Tile::Empty;
                } break;

                default: {}
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                switch (event.button.button) {
                case SDL_BUTTON_RIGHT: {
                    if (game_state.debug) {
                        game_state.tracking_projectile =
                            projectile_at_position(game_state.debug_mouse_position);

                        if (!game_state.tracking_projectile.has_value) {

                            auto index = room_index_at(game_state.debug_mouse_position);

                            Vec2i tile =
                                vec_cast<int>(
                                    (game_state.debug_mouse_position - room_row[index.unwrap].position) /
                                    TILE_SIZE);

                            if (room_row[index.unwrap].is_tile_inbounds(tile)) {
                                if (room_row[index.unwrap].tiles[tile.y][tile.x] == Tile::Empty) {
                                    game_state.state = Debug_Draw_State::Create;
                                    room_row[index.unwrap].tiles[tile.y][tile.x] = Tile::Wall;
                                } else {
                                    game_state.state = Debug_Draw_State::Delete;
                                    room_row[index.unwrap].tiles[tile.y][tile.x] = Tile::Empty;
                                }
                            }
                        }
                    }
                } break;

                case SDL_BUTTON_LEFT: {
                    entity_shoot({PLAYER_ENTITY_INDEX});
                    mixer.play_sample(shoot_sample);
                } break;
                }
            } break;

            case SDL_MOUSEBUTTONUP: {
                switch (event.button.button) {
                case SDL_BUTTON_RIGHT: {
                    game_state.state = Debug_Draw_State::Idle;
                } break;
                }
            } break;
            }
        }
        //// HANDLE INPUT END //////////////////////////////

        //// UPDATE STATE //////////////////////////////
        // TODO(#56): inertia implementation is not reusable for other entities
        if (!step_debug) {
            while (lag_sec >= SIMULATION_DELTA_TIME) {
                const float PLAYER_SPEED = 600.0f;
                const float PLAYER_ACCEL = PLAYER_SPEED * 6.0f;
                if (keyboard[SDL_SCANCODE_D]) {
                    entities[PLAYER_ENTITY_INDEX].vel.x =
                        fminf(
                            entities[PLAYER_ENTITY_INDEX].vel.x + PLAYER_ACCEL * SIMULATION_DELTA_TIME,
                            PLAYER_SPEED);
                    entities[PLAYER_ENTITY_INDEX].alive_state = Alive_State::Walking;
                } else if (keyboard[SDL_SCANCODE_A]) {
                    entities[PLAYER_ENTITY_INDEX].vel.x =
                        fmax(
                            entities[PLAYER_ENTITY_INDEX].vel.x - PLAYER_ACCEL * SIMULATION_DELTA_TIME,
                            -PLAYER_SPEED);
                    entities[PLAYER_ENTITY_INDEX].alive_state = Alive_State::Walking;
                } else {
                    const float PLAYER_STOP_THRESHOLD = 100.0f;
                    if (fabs(entities[PLAYER_ENTITY_INDEX].vel.x) > PLAYER_STOP_THRESHOLD) {
                        entities[PLAYER_ENTITY_INDEX].vel.x -=
                            sgn(entities[PLAYER_ENTITY_INDEX].vel.x) * PLAYER_ACCEL * SIMULATION_DELTA_TIME;
                    } else {
                        entities[PLAYER_ENTITY_INDEX].vel.x = 0.0f;
                    }
                    entities[PLAYER_ENTITY_INDEX].alive_state = Alive_State::Idle;
                }
                update_game_state(game_state, SIMULATION_DELTA_TIME);

                lag_sec -= SIMULATION_DELTA_TIME;
            }

            game_state.camera.pos = entities[PLAYER_ENTITY_INDEX].pos -
                vec2((float) window_w, (float) window_h) * 0.5f;
        }
        //// UPDATE STATE END //////////////////////////////

        //// RENDER //////////////////////////////
        sec(SDL_SetRenderDrawColor(renderer, 18, 8, 8, 255));
        sec(SDL_RenderClear(renderer));

        auto index = room_index_at(entities[PLAYER_ENTITY_INDEX].pos);

        if (index.unwrap == 0) {
            const SDL_Rect rect = {0, 0, (int)floorf(-game_state.camera.pos.x), window_h};
            sec(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
            sec(SDL_RenderFillRect(renderer, &rect));
        }
        // TODO(#47): there is no right border

        render_game_state(game_state, renderer);

        if (game_state.debug) {
            render_debug_overlay(game_state, renderer);
        }
        SDL_RenderPresent(renderer);
        //// RENDER END //////////////////////////////
    }

    SDL_Quit();

    return 0;
}
