template <typename F>
struct Defer
{
    Defer(F f): f(f) {}
    ~Defer() { f(); }
    F f;
};

#define CONCAT0(a, b) a##b
#define CONCAT(a, b) CONCAT0(a, b)
#define defer(body) Defer CONCAT(defer, __LINE__)([&]() { body; })

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int SIMULATION_FPS = 60;
const float SIMULATION_DELTA_TIME = 1.0f / SIMULATION_FPS;

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

Game game = {};

const char *const CONFIG_VARS_FILE_PATH = "./assets/config.vars";

int main(void)
{
    sec(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));

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
    size_t tileset_texture = texture_index_by_name("./assets/sprites/fantasy_tiles.png"_sv);

    // TODO(#9): baking assets into executable

    load_textures(renderer);
    load_samples();
    load_frame_animat_files();

    game.mixer.volume = 0.2f;
    game.keyboard = SDL_GetKeyboardState(NULL);

    game.popup.font.bitmap = load_texture_from_bmp_file(renderer, "./assets/fonts/charmap-oldschool.bmp", {0, 0, 0, 255});
    game.debug_font.bitmap = game.popup.font.bitmap;

    tile_defs[TILE_WALL].top_texture = {
        {120, 128, 16, 16},
        tileset_texture
    };
    tile_defs[TILE_WALL].bottom_texture = {
        {120, 128 + 16, 16, 16},
        tileset_texture
    };
    tile_defs[TILE_DESTROYABLE_0].top_texture = {
        {208, 176, 16, 16},
        tileset_texture,
    };
    tile_defs[TILE_DESTROYABLE_0].bottom_texture = tile_defs[TILE_DESTROYABLE_0].top_texture;
    tile_defs[TILE_DESTROYABLE_1].top_texture = {
        {208 + 16, 176, 16, 16},
        tileset_texture,
    };
    tile_defs[TILE_DESTROYABLE_1].bottom_texture = tile_defs[TILE_DESTROYABLE_1].top_texture;

    tile_defs[TILE_DESTROYABLE_2].top_texture = {
        {208, 176 + 16, 16, 16},
        tileset_texture,
    };
    tile_defs[TILE_DESTROYABLE_2].bottom_texture = tile_defs[TILE_DESTROYABLE_2].top_texture;

    tile_defs[TILE_DESTROYABLE_3].top_texture = {
        {208 + 16, 176 + 16, 16, 16},
        tileset_texture,
    };
    tile_defs[TILE_DESTROYABLE_3].bottom_texture = tile_defs[TILE_DESTROYABLE_3].top_texture;



    game.player_shoot_sample      = sample_s16_by_name("./assets/sounds/enemy_shoot-48000-decay.wav"_sv);
    game.entity_walking_animat    = frame_animat_by_name("./assets/animats/walking.txt"_sv);
    game.entity_idle_animat       = frame_animat_by_name("./assets/animats/idle.txt"_sv);
    game.entity_jump_sample1      = sample_s16_by_name("./assets/sounds/jumppp11-48000-mono.wav"_sv);
    game.entity_jump_sample2      = sample_s16_by_name("./assets/sounds/jumppp22-48000-mono.wav"_sv);
    game.projectile_poof_animat   = frame_animat_by_name("./assets/animats/plasma_pop.txt"_sv);
    game.projectile_active_animat = frame_animat_by_name("./assets/animats/plasma_bolt.txt"_sv);

#ifndef SOMETHING_RELEASE
    {
        auto result = reload_config_file(CONFIG_VARS_FILE_PATH);
        if (result.is_error) {
            println(stderr, CONFIG_VARS_FILE_PATH, ":", result.line, ": ", result.message);
            game.popup.notify(FONT_FAILURE_COLOR, "%s:%d: %s", CONFIG_VARS_FILE_PATH, result.line, result.message);
        }
    }
#endif // SOMETHING_RELEASE

    // SOUND //////////////////////////////
    SDL_AudioSpec want = {};
    want.freq = SOMETHING_SOUND_FREQ;
    want.format = SOMETHING_SOUND_FORMAT;
    want.channels = SOMETHING_SOUND_CHANNELS;
    want.samples = SOMETHING_SOUND_SAMPLES;
    want.callback = sample_mixer_audio_callback;
    want.userdata = &game.mixer;

    SDL_AudioSpec have = {};
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(
        NULL,
        0,
        &want,
        &have,
        SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    defer(SDL_CloseAudioDevice(dev));
    if (dev == 0) {
        println(stderr, "SDL pooped itself: Failed to open audio: ", SDL_GetError());
        abort();
    }
    if (have.format != want.format) {
        println(stderr, "[WARN] We didn't get expected audio format.");
        abort();
    }
    SDL_PauseAudioDevice(dev, 0);
    // SOUND END //////////////////////////////


    char room_file_path[256];
    for (size_t room_index = 0; room_index < ROOM_ROW_COUNT; ++room_index) {
        game.room_row[room_index].index = {room_index};
        game.room_row[room_index].load_file(
            file_path_of_room(
                room_file_path,
                sizeof(room_file_path),
                {room_index}));
    }

    game.reset_entities();

    bool step_debug = false;

    sec(SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND));

    Uint32 prev_ticks = SDL_GetTicks();
    float lag_sec = 0;
    Room_Index room_index_clipboard = {0};
    Toolbar debug_toolbar = {};
    while (!game.quit) {
        Uint32 curr_ticks = SDL_GetTicks();
        float elapsed_sec = (float) (curr_ticks - prev_ticks) / 1000.0f;
        prev_ticks = curr_ticks;
        lag_sec += elapsed_sec;

        {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            game.camera.width = (float) w;
            game.camera.height = (float) h;
        }

        //// HANDLE INPUT //////////////////////////////
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                game.quit = true;
            } break;

            case SDL_KEYDOWN: {
                // The reason we are not using isdigit here is because
                // isdigit(event.key.keysym.sym) maybe crash on
                // clang++ with -O0. Probably because SDL_Keycode is
                // not a char and can be bigger than char and that
                // kills isdigit if it's not optimized away?
                const auto sym = event.key.keysym.sym;
                if ('0' <= sym && sym <= '9') {
                    if (game.debug) {
                        int room_index = sym - '0' - 1;
                        if (0 <= room_index && (size_t) room_index < ROOM_ROW_COUNT) {
                            game.entities[PLAYER_ENTITY_INDEX].pos =
                                game.room_row[room_index].center();
                        }
                    }
                } else {
                    switch (event.key.keysym.sym) {
                    case SDLK_SPACE: {
                        if (!event.key.repeat) {
                            game.entity_jump({PLAYER_ENTITY_INDEX});
                        }
                    } break;

                    case SDLK_c: {
                        if (game.debug && (event.key.keysym.mod & KMOD_LCTRL)) {
                            room_index_clipboard = game.room_index_at(game.entities[PLAYER_ENTITY_INDEX].pos);
                        }
                    } break;

                    case SDLK_v: {
                        if (game.debug && (event.key.keysym.mod & KMOD_LCTRL)) {
                            game.room_row[game.room_index_at(game.entities[PLAYER_ENTITY_INDEX].pos).unwrap]
                                .copy_from(&game.room_row[room_index_clipboard.unwrap]);
                        }
                    } break;

#ifndef SOMETHING_RELEASE
                    case SDLK_F5: {
                        auto result = reload_config_file(CONFIG_VARS_FILE_PATH);
                        if (result.is_error) {
                            println(stderr, CONFIG_VARS_FILE_PATH, ":", result.line, ": ", result.message);
                            game.popup.notify(FONT_FAILURE_COLOR, "%s:%d: %s", CONFIG_VARS_FILE_PATH, result.line, result.message);
                        } else {
                            game.popup.notify(FONT_SUCCESS_COLOR, "Reloaded config file\n\n%s", CONFIG_VARS_FILE_PATH);
                        }
                    } break;
#endif  // SOMETHING_RELEASE

                    case SDLK_q: {
                        game.debug = !game.debug;
                    } break;

                    case SDLK_z: {
                        step_debug = !step_debug;
                    } break;

                    case SDLK_x: {
                        if (step_debug) {
                            game.update(SIMULATION_DELTA_TIME);
                        }
                    } break;

                    case SDLK_e: {
                        auto room_index = game.room_index_at(game.entities[PLAYER_ENTITY_INDEX].pos);
                        game.room_row[room_index.unwrap].dump_file(
                            file_path_of_room(
                                room_file_path,
                                sizeof(room_file_path),
                                room_index));
                        game.popup.notify(FONT_SUCCESS_COLOR,
                                          "Saved room %lu to `%s`",
                                          room_index.unwrap,
                                          room_file_path);
                    } break;

                    case SDLK_i: {
                        auto room_index = game.room_index_at(game.entities[PLAYER_ENTITY_INDEX].pos);
                        game.room_row[room_index.unwrap].load_file(
                            file_path_of_room(
                                room_file_path,
                                sizeof(room_file_path),
                                room_index));
                        game.popup.notify(FONT_SUCCESS_COLOR,
                                          "Load room %lu from `%s`\n",
                                          room_index.unwrap,
                                          room_file_path);
                    } break;

                    case SDLK_r: {
                        game.reset_entities();
                    } break;
                    }
                }
            } break;

            case SDL_KEYUP: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    if (!event.key.repeat) {
                        game.entity_jump({PLAYER_ENTITY_INDEX});
                    }
                } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                game.debug_mouse_position =
                    game.camera.to_world(vec_cast<float>(vec2(event.motion.x, event.motion.y)));
                game.collision_probe = game.debug_mouse_position;

                auto index = game.room_index_at(game.collision_probe);
                game.room_row[index.unwrap].resolve_point_collision(&game.collision_probe);

                Vec2i tile = vec_cast<int>((game.debug_mouse_position - game.room_row[index.unwrap].position()) / TILE_SIZE);
                switch (game.state) {
                case Debug_Draw_State::Create: {
                    if (game.room_row[index.unwrap].is_tile_inbounds(tile))
                        game.room_row[index.unwrap].tiles[tile.y][tile.x] = TILE_WALL;
                } break;

                case Debug_Draw_State::Delete: {
                    if (game.room_row[index.unwrap].is_tile_inbounds(tile))
                        game.room_row[index.unwrap].tiles[tile.y][tile.x] = TILE_EMPTY;
                } break;

                case Debug_Draw_State::Idle:
                default: {}
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                switch (event.button.button) {
                case SDL_BUTTON_RIGHT: {
                    if (game.debug) {
                        game.tracking_projectile =
                            game.projectile_at_position(game.debug_mouse_position);

                        if (!game.tracking_projectile.has_value) {
                            switch (debug_toolbar.current_button) {
                            case Toolbar::Tiles: {
                                auto index = game.room_index_at(game.debug_mouse_position);

                                Vec2i tile =
                                    vec_cast<int>(
                                        (game.debug_mouse_position - game.room_row[index.unwrap].position()) /
                                        TILE_SIZE);

                                if (game.room_row[index.unwrap].is_tile_inbounds(tile)) {
                                    if (game.room_row[index.unwrap].tiles[tile.y][tile.x] == TILE_EMPTY) {
                                        game.state = Debug_Draw_State::Create;
                                        game.room_row[index.unwrap].tiles[tile.y][tile.x] = TILE_WALL;
                                    } else {
                                        game.state = Debug_Draw_State::Delete;
                                        game.room_row[index.unwrap].tiles[tile.y][tile.x] = TILE_EMPTY;
                                    }
                                }
                            } break;

                            case Toolbar::Heals: {
                                game.spawn_health_at_mouse();
                            } break;

                            case Toolbar::Button_Count:
                            default: {}
                            }
                        }
                    }
                } break;

                case SDL_BUTTON_LEFT: {
                    if (!debug_toolbar.handle_click_at({(float)event.button.x, (float)event.button.y},
                                                       game.camera)) {
                        game.entity_shoot({PLAYER_ENTITY_INDEX});
                    }
                } break;
                }
            } break;

            case SDL_MOUSEBUTTONUP: {
                switch (event.button.button) {
                case SDL_BUTTON_RIGHT: {
                    game.state = Debug_Draw_State::Idle;
                } break;
                }
            } break;
            }
        }
        //// HANDLE INPUT END //////////////////////////////

        //// UPDATE STATE //////////////////////////////
        if (!step_debug) {
            SDL_Delay(1);
            while (lag_sec >= SIMULATION_DELTA_TIME) {
                game.update(SIMULATION_DELTA_TIME);
                lag_sec -= SIMULATION_DELTA_TIME;
            }
        }
        //// UPDATE STATE END //////////////////////////////

        //// RENDER //////////////////////////////
        sec(SDL_SetRenderDrawColor(
                renderer,
                BACKGROUND_COLOR.r,
                BACKGROUND_COLOR.g,
                BACKGROUND_COLOR.b,
                BACKGROUND_COLOR.a));
        sec(SDL_RenderClear(renderer));
        game.render(renderer);
        debug_toolbar.render(renderer, game.camera);

        if (game.debug) {
            game.render_debug_overlay(renderer);
        }
        SDL_RenderPresent(renderer);
        //// RENDER END //////////////////////////////
    }

    SDL_Quit();

    return 0;
}
