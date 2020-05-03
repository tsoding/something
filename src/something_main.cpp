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

Game_State game_state = {};

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
    SDL_Texture *tileset_texture = load_texture_from_png_file(
        renderer,
        "assets/sprites/fantasy_tiles.png");

    // TODO(#9): baking assets into executable

    load_spritesheets(renderer);

    stec(TTF_Init());
    const int DEBUG_FONT_SIZE = 32;


    game_state.mixer.volume = 0.2f;
    game_state.keyboard = SDL_GetKeyboardState(NULL);
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
    game_state.player_shoot_sample      = load_wav_as_sample_s16("./assets/sounds/enemy_shoot-48000-decay.wav");
    game_state.entity_walking_animat    = load_animat_file("./assets/animats/walking.txt");
    game_state.entity_idle_animat       = load_animat_file("./assets/animats/idle.txt");
    game_state.entity_jump_sample1      = load_wav_as_sample_s16("./assets/sounds/jumppp11-48000-mono.wav");
    game_state.entity_jump_sample2      = load_wav_as_sample_s16("./assets/sounds/jumppp22-48000-mono.wav");
    game_state.projectile_poof_animat   = load_animat_file("./assets/animats/plasma_pop.txt");
    game_state.projectile_active_animat = load_animat_file("./assets/animats/plasma_bolt.txt");

    // SOUND //////////////////////////////
    SDL_AudioSpec want = {};
    want.freq = SOMETHING_SOUND_FREQ;
    want.format = SOMETHING_SOUND_FORMAT;
    want.channels = SOMETHING_SOUND_CHANNELS;
    want.samples = SOMETHING_SOUND_SAMPLES;
    want.callback = sample_mixer_audio_callback;
    want.userdata = &game_state.mixer;

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
        room_row[room_index].position = {(float) room_index * ROOM_BOUNDARY.w, 0};
        room_row[room_index].load_file(
            file_path_of_room(
                room_file_path,
                sizeof(room_file_path),
                {room_index}));
    }

    game_state.reset_entities();

    SDL_SetWindowGrab(window, game_state.debug ? SDL_FALSE : SDL_TRUE);

    bool step_debug = false;

    sec(SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND));

    Uint32 prev_ticks = SDL_GetTicks();
    float lag_sec = 0;
    Room_Index room_index_clipboard = {0};
    while (!game_state.quit) {
        Uint32 curr_ticks = SDL_GetTicks();
        float elapsed_sec = (float) (curr_ticks - prev_ticks) / 1000.0f;
        prev_ticks = curr_ticks;
        lag_sec += elapsed_sec;

        {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            game_state.camera.width = (float) w;
            game_state.camera.height = (float) h;
        }

        //// HANDLE INPUT //////////////////////////////
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                game_state.quit = true;
            } break;

            case SDL_KEYDOWN: {
                // The reason we are not using isdigit here is because
                // isdigit(event.key.keysym.sym) maybe crash on
                // clang++ with -O0. Probably because SDL_Keycode is
                // not a char and can be bigger than char and that
                // kills isdigit if it's not optimized away?
                const auto sym = event.key.keysym.sym;
                if ('0' <= sym && sym <= '9') {
                    if (game_state.debug) {
                        int room_index = sym - '0' - 1;
                        if (0 <= room_index && (size_t) room_index < ROOM_ROW_COUNT) {
                            auto room_center =
                                vec2(ROOM_BOUNDARY.w * 0.5f, ROOM_BOUNDARY.h * 0.5f) +
                                room_row[room_index].position;
                            game_state.entities[PLAYER_ENTITY_INDEX].pos = room_center;
                        }
                    }
                } else {
                    switch (event.key.keysym.sym) {
                    case SDLK_SPACE: {
                        if (!event.key.repeat) {
                            game_state.entity_jump({PLAYER_ENTITY_INDEX});
                        }
                    } break;

                    case SDLK_c: {
                        if (game_state.debug && (event.key.keysym.mod & KMOD_LCTRL)) {
                            room_index_clipboard = room_index_at(game_state.entities[PLAYER_ENTITY_INDEX].pos);
                        }
                    } break;

                    case SDLK_v: {
                        if (game_state.debug && (event.key.keysym.mod & KMOD_LCTRL)) {
                            room_row[room_index_at(game_state.entities[PLAYER_ENTITY_INDEX].pos).unwrap]
                                .copy_from(&room_row[room_index_clipboard.unwrap]);
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
                            game_state.update(SIMULATION_DELTA_TIME);
                        }
                    } break;

                    case SDLK_e: {
                        auto room_index = room_index_at(game_state.entities[PLAYER_ENTITY_INDEX].pos);
                        room_row[room_index.unwrap].dump_file(
                            file_path_of_room(
                                room_file_path,
                                sizeof(room_file_path),
                                room_index));
                        fprintf(stderr, "Saved room %lu to `%s`\n",
                                room_index.unwrap, room_file_path);
                    } break;

                    case SDLK_i: {
                        auto room_index = room_index_at(game_state.entities[PLAYER_ENTITY_INDEX].pos);
                        room_row[room_index.unwrap].load_file(
                            file_path_of_room(
                                room_file_path,
                                sizeof(room_file_path),
                                room_index));
                        fprintf(stderr, "Load room %lu from `%s`\n",
                                room_index.unwrap, room_file_path);
                    } break;

                    case SDLK_r: {
                        game_state.reset_entities();
                    } break;
                    }
                }
            } break;

            case SDL_KEYUP: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    if (!event.key.repeat) {
                        game_state.entity_jump({PLAYER_ENTITY_INDEX});
                    }
                } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                game_state.debug_mouse_position =
                    game_state.camera.to_world(vec_cast<float>(vec2(event.motion.x, event.motion.y)));
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
                            game_state.projectile_at_position(game_state.debug_mouse_position);

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
                    game_state.entity_shoot({PLAYER_ENTITY_INDEX});

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
        if (!step_debug) {
            while (lag_sec >= SIMULATION_DELTA_TIME) {

                game_state.update(SIMULATION_DELTA_TIME);
                lag_sec -= SIMULATION_DELTA_TIME;
            }
        }
        //// UPDATE STATE END //////////////////////////////

        //// RENDER //////////////////////////////
        sec(SDL_SetRenderDrawColor(renderer, 18, 8, 8, 255));
        sec(SDL_RenderClear(renderer));

        auto index = room_index_at(game_state.entities[PLAYER_ENTITY_INDEX].pos);

        if (index.unwrap == 0) {
            const SDL_Rect rect = {
                0, 0,
                (int)floorf(-game_state.camera.pos.x),
                (int)floorf(game_state.camera.height)};
            sec(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
            sec(SDL_RenderFillRect(renderer, &rect));
        }
        // TODO(#47): there is no right border

        game_state.render(renderer);

        if (game_state.debug) {
            game_state.render_debug_overlay(renderer);
        }
        SDL_RenderPresent(renderer);
        //// RENDER END //////////////////////////////
    }

    SDL_Quit();

    return 0;
}
