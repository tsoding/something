#include "something_fmw.hpp"
#include "something_assets.hpp"

const int SIMULATION_FPS = 60;
const float SIMULATION_DELTA_TIME = 1.0f / SIMULATION_FPS;

Game game = {};

Dynamic_Array<Dynamic_Array<char>> load_room_files_from_dir(const char *room_dir_path)
{
    Dynamic_Array<Dynamic_Array<char>> room_files = {};

    DIR *rooms_dir = opendir(room_dir_path);
    defer(closedir(rooms_dir));
    if (rooms_dir == NULL) {
        println(stderr, "Can't open asset folder: ", room_dir_path);
        abort();
    }

    for (struct dirent *d = readdir(rooms_dir);
         d != NULL;
         d = readdir(rooms_dir))
    {
        if (*d->d_name == '.') continue;
        Dynamic_Array<char> room_file = {};
        room_file.concat(room_dir_path, strlen(room_dir_path));
        room_file.concat(d->d_name, strlen(d->d_name));
        room_file.push('\0');
        room_files.push(room_file);
    }

    return room_files;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    sec(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));

    SDL_Window *window =
        sec(SDL_CreateWindow(
                "Something",
                0, 0, (int) SCREEN_WIDTH, (int) SCREEN_HEIGHT,
                SDL_WINDOW_RESIZABLE));

    SDL_Renderer *renderer =
        sec(SDL_CreateRenderer(
                window, -1,
                SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

    assets.load_conf(renderer, "./assets/assets.conf");

    SDL_StopTextInput();

    sec(SDL_RenderSetLogicalSize(renderer,
                           (int) SCREEN_WIDTH,
                           (int) SCREEN_HEIGHT));

    // TODO(#8): replace fantasy_tiles.png with our own assets
    auto tileset_texture = FANTASY_TEXTURE_INDEX;

    game.window = window;
    game.renderer = renderer;

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
    tile_defs[TILE_DIRT_0].top_texture = {
        {208, 176, 16, 16},
        tileset_texture,
    };
    tile_defs[TILE_DIRT_0].bottom_texture = tile_defs[TILE_DIRT_0].top_texture;
    tile_defs[TILE_DIRT_1].top_texture = {
        {208 + 16, 176, 16, 16},
        tileset_texture,
    };
    tile_defs[TILE_DIRT_1].bottom_texture = tile_defs[TILE_DIRT_1].top_texture;

    tile_defs[TILE_DIRT_2].top_texture = {
        {208, 176 + 16, 16, 16},
        tileset_texture,
    };
    tile_defs[TILE_DIRT_2].bottom_texture = tile_defs[TILE_DIRT_2].top_texture;

    tile_defs[TILE_DIRT_3].top_texture = {
        {208 + 16, 176 + 16, 16, 16},
        tileset_texture,
    };
    tile_defs[TILE_DIRT_3].bottom_texture = tile_defs[TILE_DIRT_3].top_texture;

    tile_defs[TILE_ICE_0].bottom_texture = {
        {0, 0, 64, 64},
        ICE_BLOCK_TEXTURE_INDEX
    };
    tile_defs[TILE_ICE_0].top_texture = tile_defs[TILE_ICE_0].bottom_texture;

    tile_defs[TILE_ICE_1].bottom_texture = {
        {64, 0, 64, 64},
        ICE_BLOCK_TEXTURE_INDEX
    };
    tile_defs[TILE_ICE_1].top_texture = tile_defs[TILE_ICE_1].bottom_texture;

    tile_defs[TILE_ICE_2].bottom_texture = {
        {128, 0, 64, 64},
        ICE_BLOCK_TEXTURE_INDEX
    };
    tile_defs[TILE_ICE_2].top_texture = tile_defs[TILE_ICE_2].bottom_texture;

    tile_defs[TILE_ICE_3].bottom_texture = {
        {192, 0, 64, 64},
        ICE_BLOCK_TEXTURE_INDEX
    };
    tile_defs[TILE_ICE_3].top_texture = tile_defs[TILE_ICE_3].bottom_texture;

    game.background.layers[0] = sprite_from_texture_index(BACKGROUND_LIGHTS_TEXTURE_INDEX);
    game.background.layers[1] = sprite_from_texture_index(BACKGROUND_MIDDLE_TEXTURE_INDEX);
    game.background.layers[2] = sprite_from_texture_index(BACKGROUND_FRONT_TEXTURE_INDEX);

#ifndef SOMETHING_RELEASE
    {
        auto result = reload_config_file(VARS_CONF_FILE_PATH);
        if (result.is_error) {
            println(stderr, VARS_CONF_FILE_PATH, ":", result.line, ": ", result.message);
            game.popup.notify(FONT_FAILURE_COLOR, "%s:%d: %s", VARS_CONF_FILE_PATH, result.line, result.message);
        }
    }

    auto fmw = fmw_init(VARS_CONF_FILE_PATH);
#endif // SOMETHING_RELEASE

    static_assert(DEBUG_TOOLBAR_COUNT <= TOOLBAR_BUTTONS_CAPACITY);
    game.debug_toolbar.buttons_count = DEBUG_TOOLBAR_COUNT;

    game.debug_toolbar.buttons[DEBUG_TOOLBAR_TILES].icon = tile_defs[TILE_WALL].top_texture;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_TILES].tooltip = "Edit walls"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_TILES].tool.type = Tool_Type::Tile;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_TILES].tool.tile.tile = TILE_WALL;

    game.debug_toolbar.buttons[DEBUG_TOOLBAR_DESTROYABLE].icon = tile_defs[TILE_DIRT_0].top_texture;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_DESTROYABLE].tooltip = "Destroyable Tile"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_DESTROYABLE].tool.type = Tool_Type::Tile;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_DESTROYABLE].tool.tile.tile = TILE_DIRT_0;

    game.debug_toolbar.buttons[DEBUG_TOOLBAR_HEALS].icon = sprite_from_texture_index(
        HEALTH_ITEM_TEXTURE_INDEX);
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_HEALS].tooltip = "Add health items"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_HEALS].tool.type = Tool_Type::Item;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_HEALS].tool.item.item = make_health_item(vec2(0.0f, 0.0f));

    assert(ENEMY_IDLE_ANIMAT.frame_count > 0);
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ENEMIES].icon = ENEMY_IDLE_ANIMAT.frames[0];
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ENEMIES].tooltip = "Add enemies"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ENEMIES].tool.type = Tool_Type::Entity;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ENEMIES].tool.entity.entity = enemy_entity(vec2(0.0f, 0.0f));

    game.debug_toolbar.buttons[DEBUG_TOOLBAR_DIRT].icon = tile_defs[TILE_DIRT_0].top_texture;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_DIRT].tooltip = "Add dirt block items"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_DIRT].tool.type = Tool_Type::Item;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_DIRT].tool.item.item = make_dirt_block_item(vec2(0.0f, 0.0f));

    game.debug_toolbar.buttons[DEBUG_TOOLBAR_GOLEM].icon = sprite_from_texture_index(
        DIRT_GOLEM_TEXTURE_INDEX);
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_GOLEM].tooltip = "Add golem enemy"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_GOLEM].tool.type = Tool_Type::Entity;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_GOLEM].tool.entity.entity = golem_entity(vec2(0.0f, 0.0f));

    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_BLOCK].icon = tile_defs[TILE_ICE_0].top_texture;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_BLOCK].tooltip = "Add ice blocks"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_BLOCK].tool.type = Tool_Type::Tile;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_BLOCK].tool.tile.tile = TILE_ICE_0;

    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_ITEM].icon = tile_defs[TILE_ICE_0].top_texture;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_ITEM].tooltip = "Add ice items"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_ITEM].tool.type = Tool_Type::Item;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_ITEM].tool.item.item = make_ice_block_item(vec2(0.0f, 0.0f));

    assert(ICE_GOLEM_WALKING_ANIMAT.frame_count > 0);
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_GOLEM].icon = ICE_GOLEM_WALKING_ANIMAT.frames[0];
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_GOLEM].tooltip = "Add ice golem enemy"_sv;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_GOLEM].tool.type = Tool_Type::Entity;
    game.debug_toolbar.buttons[DEBUG_TOOLBAR_ICE_GOLEM].tool.entity.entity = ice_golem_entity(vec2(0.0f, 0.0f));

    // TODO(#234): Separate kind of fire projectiles that can destroy ice blocks
    // TODO(#235): Separate kind of water projectiles that can destroy dirt blocks

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
        0);
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

    auto room_files = load_room_files_from_dir("./assets/rooms/");

    const int PADDING = 1;
    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            const auto coord = vec2(x * (ROOM_WIDTH + PADDING), y * (ROOM_HEIGHT + PADDING));
            const size_t room_index = rand() % room_files.size;
            game.grid.load_room_from_file(room_files.data[room_index].data, coord);
            game.add_camera_lock(rect(coord, ROOM_WIDTH, ROOM_HEIGHT));
        }
    }

    sec(SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND));

    Uint32 prev_ticks = SDL_GetTicks();
    float lag_sec = 0;
    float next_sec = 0;
    size_t frames_of_current_second = 0;
    size_t fps = 0;
    while (!game.quit) {
        Uint32 curr_ticks = SDL_GetTicks();
        float elapsed_sec = (float) (curr_ticks - prev_ticks) / 1000.0f;
        if(game.fps_debug) {
            game.frame_delays[game.frame_delays_begin] = elapsed_sec;
            game.frame_delays_begin = (game.frame_delays_begin + 1) % FPS_BARS_COUNT;
        }

        frames_of_current_second += 1;
        next_sec += elapsed_sec;

        if (next_sec >= 1) {
            fps = frames_of_current_second;
            next_sec -= 1.0f;
            frames_of_current_second = 0;
        }

        prev_ticks = curr_ticks;
        lag_sec += elapsed_sec;

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

                case SDLK_F6: {
                    // NOTE: it is important to clean all of the
                    // samples from the mixer before reloading the
                    // assets, because after assets are reloaded any
                    // pointers stored in the mixer could be
                    // invalidated.
                    game.mixer.clean();
                    assets.load_conf(renderer, "./assets/assets.conf");
                    game.popup.notify(FONT_SUCCESS_COLOR, "Reloaded assets file");
                } break;
                }
            } break;
            }

            game.handle_event(&event);
        }

#ifndef SOMETHING_RELEASE
        if (fmw_poll(fmw)) {
            auto result = reload_config_file(VARS_CONF_FILE_PATH);
            if (result.is_error) {
                println(stderr, VARS_CONF_FILE_PATH, ":", result.line, ": ", result.message);
                game.popup.notify(FONT_FAILURE_COLOR, "%s:%d: %s", VARS_CONF_FILE_PATH, result.line, result.message);
            } else {
                game.popup.notify(FONT_SUCCESS_COLOR, "Reloaded config file\n\n%s", VARS_CONF_FILE_PATH);
            }
        }
#endif // SOMETHING_RELEASE
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
        const SDL_Color background_color = rgba_to_sdl(BACKGROUND_COLOR);
        sec(SDL_SetRenderDrawColor(
                renderer,
                background_color.r,
                background_color.g,
                background_color.b,
                background_color.a));
        sec(SDL_RenderClear(renderer));
        const SDL_Color canvas_background_color = rgba_to_sdl(CANVAS_BACKGROUND_COLOR);
        sec(SDL_SetRenderDrawColor(
                renderer,
                canvas_background_color.r,
                canvas_background_color.g,
                canvas_background_color.b,
                canvas_background_color.a));
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
