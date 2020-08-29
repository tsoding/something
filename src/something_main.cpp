const int SIMULATION_FPS = 60;
const float SIMULATION_DELTA_TIME = 1.0f / SIMULATION_FPS;

template <typename T>
void print1(FILE *stream, Vec2<T> v)
{
    print(stream, '(', v.x, ',', v.y, ')');
}

Game game = {};

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

    sec(SDL_RenderSetLogicalSize(renderer,
                                 SCREEN_WIDTH,
                                 SCREEN_HEIGHT));

    // TODO(#8): replace fantasy_tiles.png with our own assets
    size_t tileset_texture = texture_index_by_name("./assets/sprites/fantasy_tiles.png"_sv);

    load_textures(renderer);
    load_samples();
    load_frame_animat_files();

    game.mixer.volume = 0.2f;
    game.keyboard = SDL_GetKeyboardState(NULL);

    game.popup.font.bitmap = load_texture_from_bmp_file(renderer, "./assets/fonts/charmap-oldschool.bmp", {0, 0, 0, 255});
    game.debug_font.bitmap = game.popup.font.bitmap;

    // TODO(#119): move tiles srcrect dimention to config.vars
    //   That may require add a new type to the config file.
    //   Might be a good opportunity to simplify adding new types to the system.
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
    /// {"./assets/sounds/Fallbig1.wav", {}},
    /// {"./assets/sounds/Hurt_Old.wav", {}},
    game.damage_enemy_sample      = sample_s16_by_name("./assets/sounds/Hurt_Old.wav"_sv);
    game.kill_enemy_sample        = sample_s16_by_name("./assets/sounds/Fallbig1.wav"_sv);

#ifndef SOMETHING_RELEASE
    {
        auto result = reload_config_file(CONFIG_VARS_FILE_PATH);
        if (result.is_error) {
            println(stderr, CONFIG_VARS_FILE_PATH, ":", result.line, ": ", result.message);
            game.popup.notify(FONT_FAILURE_COLOR, "%s:%d: %s", CONFIG_VARS_FILE_PATH, result.line, result.message);
        }
    }
#endif // SOMETHING_RELEASE

    static_assert(DEBUG_TOOLBAR_COUNT <= TOOLBAR_BUTTONS_CAPACITY);
    game.debug_toolbar.buttons_count = DEBUG_TOOLBAR_COUNT;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_TILES].icon = tile_defs[TILE_WALL].top_texture;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_TILES].tooltip = "Edit walls"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_HEALS].icon = sprite_from_texture_index(
        texture_index_by_name(
            ITEM_HEALTH_TEXTURE));
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_HEALS].tooltip = "Add health items"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ENEMIES].icon = game.entity_idle_animat.frames[0];
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ENEMIES].tooltip = "Add enemies"_sv;

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

    game.reset_entities();
    char filepath[256];
    const int PADDING = 1;
    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            snprintf(filepath, sizeof(filepath), "assets/rooms/room-%d.bin", rand() % 3);
            auto coord = vec2(x * (ROOM_WIDTH + PADDING), y * (ROOM_HEIGHT + PADDING));
            game.grid.load_room_from_file(filepath, coord);
            game.add_camera_lock(rect(coord, ROOM_WIDTH, ROOM_HEIGHT));
        }
    }

    sec(SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND));

    Uint32 prev_ticks = SDL_GetTicks();
    float lag_sec = 0;
    float fps = 60.0f;
    while (!game.quit) {
        Uint32 curr_ticks = SDL_GetTicks();
        float elapsed_sec = (float) (curr_ticks - prev_ticks) / 1000.0f;
        prev_ticks = curr_ticks;
        lag_sec += elapsed_sec;
        float instant_fps = 1.0f / elapsed_sec;
        if (elapsed_sec < 1.0e-5) instant_fps = 60.0f;
        fps = fps * 0.99f + instant_fps * 0.01f;

        //// HANDLE INPUT //////////////////////////////
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_x: {
                    if (game.step_debug) {
                        game.update(SIMULATION_DELTA_TIME);
                    }
                } break;
                }
            } break;
            }

            game.handle_event(&event);
        }
        //// HANDLE INPUT END //////////////////////////////

        //// UPDATE STATE //////////////////////////////
        if (!game.step_debug) {
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
        sec(SDL_SetRenderDrawColor(
                renderer,
                CANVAS_BACKGROUND_COLOR.r,
                CANVAS_BACKGROUND_COLOR.g,
                CANVAS_BACKGROUND_COLOR.b,
                CANVAS_BACKGROUND_COLOR.a));
        {
            SDL_Rect canvas = {0, 0, (int) floorf(SCREEN_WIDTH), (int) floorf(SCREEN_HEIGHT)};
            SDL_RenderFillRect(renderer, &canvas);
        }
        game.render(renderer);
        if (game.debug) {
            game.render_debug_overlay(renderer, fps);
        }
        SDL_RenderPresent(renderer);
        //// RENDER END //////////////////////////////
    }

    SDL_Quit();

    return 0;
}
