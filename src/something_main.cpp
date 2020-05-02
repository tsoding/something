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

const size_t ENEMY_ENTITY_INDEX_OFFSET = 1;
const size_t PLAYER_ENTITY_INDEX = 0;

const size_t ENTITIES_COUNT = 69;
const size_t PROJECTILES_COUNT = 69;

const float PROJECTILE_TRACKING_PADDING = 50.0f;

struct Entity_Index
{
    size_t unwrap;
};

struct Projectile_Index
{
    size_t unwrap;
};

enum class Projectile_State
{
    Ded = 0,
    Active,
    Poof
};

const char *projectile_state_as_cstr(Projectile_State state)
{
    switch (state) {
    case Projectile_State::Ded: return "Ded";
    case Projectile_State::Active: return "Active";
    case Projectile_State::Poof: return "Poof";
    }

    assert(0 && "Incorrect Projectile_State");
}

struct Projectile
{
    Entity_Index shooter;
    Projectile_State state;
    Vec2f pos;
    Vec2f vel;
    Frame_Animat active_animat;
    Frame_Animat poof_animat;
    float lifetime;
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

    Entity entities[ENTITIES_COUNT];
    Projectile projectiles[PROJECTILES_COUNT];

    void update(float dt)
    {
        // Update Player's gun direction
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        entities[PLAYER_ENTITY_INDEX].point_gun_at(
            camera.to_world(vec2((float) mouse_x, (float) mouse_y)));

        // Enemy AI
        if (!debug) {
            for (size_t i = 0; i < ROOM_ROW_COUNT - 1; ++i) {
                size_t player_index = room_index_at(entities[PLAYER_ENTITY_INDEX].pos).unwrap;
                size_t enemy_index = room_index_at(entities[ENEMY_ENTITY_INDEX_OFFSET + i].pos).unwrap;
                if (player_index == enemy_index) {
                    entities[ENEMY_ENTITY_INDEX_OFFSET + i].point_gun_at(
                        entities[PLAYER_ENTITY_INDEX].pos);
                    entity_shoot({ENEMY_ENTITY_INDEX_OFFSET + i});
                }
            }
        }

        // Update All Entities
        for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
            entities[i].update(gravity, dt);
        }

        // Update All Projectiles
        update_projectiles(dt);

        // Entities/Projectiles interaction
        for (size_t index = 0; index < PROJECTILES_COUNT; ++index) {
            auto projectile = projectiles + index;
            if (projectile->state != Projectile_State::Active) continue;

            for (size_t entity_index = 0;
                 entity_index < ENTITIES_COUNT;
                 ++entity_index)
            {
                auto entity = entities + entity_index;

                if (entity->state != Entity_State::Alive) continue;
                if (entity_index == projectile->shooter.unwrap) continue;

                if (rect_contains_vec2(entity->hitbox_world(), projectile->pos)) {
                    projectile->state = Projectile_State::Poof;
                    projectile->poof_animat.frame_current = 0;
                    entity->kill();
                }
            }
        }
    }

    void render(SDL_Renderer *renderer)
    {
        auto index = room_index_at(entities[PLAYER_ENTITY_INDEX].pos);

        const int NEIGHBOR_ROOM_DIM_ALPHA = 200;

        if (index.unwrap > 0) {
            room_row[index.unwrap - 1].render(
                renderer,
                camera,
                ground_grass_texture,
                ground_texture,
                {0, 0, 0, NEIGHBOR_ROOM_DIM_ALPHA});
        }

        room_row[index.unwrap].render(
            renderer,
            camera,
            ground_grass_texture,
            ground_texture);

        if (index.unwrap + 1 < (int) ROOM_ROW_COUNT) {
            room_row[index.unwrap + 1].render(
                renderer,
                camera,
                ground_grass_texture,
                ground_texture,
                {0, 0, 0, NEIGHBOR_ROOM_DIM_ALPHA});
        }

        for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
            entities[i].render(renderer, camera);
        }

        render_projectiles(renderer, camera);
    }

    void entity_shoot(Entity_Index entity_index)
    {
        assert(entity_index.unwrap < ENTITIES_COUNT);

        Entity *entity = &entities[entity_index.unwrap];

        if (entity->state != Entity_State::Alive) return;
        if (entity->cooldown_weapon > 0) return;

        const float PROJECTILE_SPEED = 1200.0f;

        const int ENTITY_COOLDOWN_WEAPON = 7;
        spawn_projectile(
            entity->pos,
            entity->gun_dir * PROJECTILE_SPEED,
            entity_index);
        entity->cooldown_weapon = ENTITY_COOLDOWN_WEAPON;
    }

    void inplace_spawn_entity(Entity_Index index,
                              Frame_Animat walking,
                              Frame_Animat idle,
                              Sample_S16 jump_sample1,
                              Sample_S16 jump_sample2,
                              Vec2f pos)
    {
        const int ENTITY_TEXBOX_SIZE = 64;
        const int ENTITY_HITBOX_SIZE = ENTITY_TEXBOX_SIZE - 20;

        const Rectf texbox_local = {
            - (ENTITY_TEXBOX_SIZE / 2), - (ENTITY_TEXBOX_SIZE / 2),
            ENTITY_TEXBOX_SIZE, ENTITY_TEXBOX_SIZE
        };
        const Rectf hitbox_local = {
            - (ENTITY_HITBOX_SIZE / 2), - (ENTITY_HITBOX_SIZE / 2),
            ENTITY_HITBOX_SIZE, ENTITY_HITBOX_SIZE
        };

        const float POOF_DURATION = 0.2f;

        memset(entities + index.unwrap, 0, sizeof(Entity));
        entities[index.unwrap].state = Entity_State::Alive;
        entities[index.unwrap].alive_state = Alive_State::Idle;
        entities[index.unwrap].texbox_local = texbox_local;
        entities[index.unwrap].hitbox_local = hitbox_local;
        entities[index.unwrap].pos = pos;
        entities[index.unwrap].gun_dir = vec2(1.0f, 0.0f);
        entities[index.unwrap].poof.duration = POOF_DURATION;

        entities[index.unwrap].walking = walking;
        entities[index.unwrap].idle = idle;

        entities[index.unwrap].prepare_for_jump_animat.begin = 0.0f;
        entities[index.unwrap].prepare_for_jump_animat.end = 0.2f;
        entities[index.unwrap].prepare_for_jump_animat.duration = 0.2f;

        entities[index.unwrap].jump_animat.rubber_animats[0].begin = 0.2f;
        entities[index.unwrap].jump_animat.rubber_animats[0].end = -0.2f;
        entities[index.unwrap].jump_animat.rubber_animats[0].duration = 0.1f;

        entities[index.unwrap].jump_animat.rubber_animats[1].begin = -0.2f;
        entities[index.unwrap].jump_animat.rubber_animats[1].end = 0.0f;
        entities[index.unwrap].jump_animat.rubber_animats[1].duration = 0.2f;

        entities[index.unwrap].jump_samples[0] = jump_sample1;
        entities[index.unwrap].jump_samples[1] = jump_sample2;
    }

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
                                 room_row[i + 1].center());
        }
    }

    void spawn_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter)
    {
        const float PROJECTILE_LIFETIME = 5.0f;
        for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
            if (projectiles[i].state == Projectile_State::Ded) {
                projectiles[i].state = Projectile_State::Active;
                projectiles[i].pos = pos;
                projectiles[i].vel = vel;
                projectiles[i].shooter = shooter;
                projectiles[i].lifetime = PROJECTILE_LIFETIME;
                return;
            }
        }
    }

    void render_debug_overlay(SDL_Renderer *renderer)
    {
        sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));

        const float COLLISION_PROBE_SIZE = 10.0f;
        const auto collision_probe_rect = rect(
            camera.to_screen(collision_probe - COLLISION_PROBE_SIZE),
            COLLISION_PROBE_SIZE * 2, COLLISION_PROBE_SIZE * 2);
        {
            auto rect = rectf_for_sdl(collision_probe_rect);
            sec(SDL_RenderFillRect(renderer, &rect));
        }

        auto index = room_index_at(debug_mouse_position);
        auto room_boundary_screen =
            camera.to_screen(ROOM_BOUNDARY + vec2((float) index.unwrap * ROOM_BOUNDARY.w, 1.0f));
        {
            auto rect = rectf_for_sdl(room_boundary_screen);
            sec(SDL_RenderDrawRect(renderer, &rect));
        }

        const float PADDING = 10.0f;
        // TODO(#38): FPS display is broken
        const SDL_Color GENERAL_DEBUG_COLOR = {255, 0, 0, 255};
        displayf(renderer, debug_font,
                 GENERAL_DEBUG_COLOR, vec2(PADDING, PADDING),
                 "FPS: %d", 60);
        displayf(renderer, debug_font,
                 GENERAL_DEBUG_COLOR, vec2(PADDING, 50 + PADDING),
                 "Mouse Position: (%.4f, %.4f)",
                 debug_mouse_position.x,
                 debug_mouse_position.y);
        displayf(renderer, debug_font,
                 GENERAL_DEBUG_COLOR, vec2(PADDING, 2 * 50 + PADDING),
                 "Collision Probe: (%.4f, %.4f)",
                 collision_probe.x,
                 collision_probe.y);
        displayf(renderer, debug_font,
                 GENERAL_DEBUG_COLOR, vec2(PADDING, 3 * 50 + PADDING),
                 "Projectiles: %d",
                 count_alive_projectiles());
        displayf(renderer, debug_font,
                 GENERAL_DEBUG_COLOR, vec2(PADDING, 4 * 50 + PADDING),
                 "Player position: (%.4f, %.4f)",
                 entities[PLAYER_ENTITY_INDEX].pos.x,
                 entities[PLAYER_ENTITY_INDEX].pos.y);
        displayf(renderer, debug_font,
                 GENERAL_DEBUG_COLOR, vec2(PADDING, 5 * 50 + PADDING),
                 "Player velocity: (%.4f, %.4f)",
                 entities[PLAYER_ENTITY_INDEX].vel.x,
                 entities[PLAYER_ENTITY_INDEX].vel.y);

        const auto minimap_position = vec2(PADDING, 6 * 50 + PADDING);
        render_room_row_minimap(renderer, minimap_position);
        render_entity_on_minimap(
            renderer,
            vec2((float) minimap_position.x, (float) minimap_position.y),
            entities[PLAYER_ENTITY_INDEX].pos);


        if (tracking_projectile.has_value) {
            auto projectile = projectiles[tracking_projectile.unwrap.unwrap];
            const float SECOND_COLUMN_OFFSET = 700.0f;
            const SDL_Color TRACKING_DEBUG_COLOR = {255, 255, 0, 255};
            displayf(renderer, debug_font,
                     TRACKING_DEBUG_COLOR, vec2(PADDING + SECOND_COLUMN_OFFSET, PADDING),
                     "State: %s", projectile_state_as_cstr(projectile.state));
            displayf(renderer, debug_font,
                     TRACKING_DEBUG_COLOR, vec2(PADDING + SECOND_COLUMN_OFFSET, 50 + PADDING),
                     "Position: (%.4f, %.4f)",
                     projectile.pos.x, projectile.pos.y);
            displayf(renderer, debug_font,
                     TRACKING_DEBUG_COLOR, vec2(PADDING + SECOND_COLUMN_OFFSET, 2 * 50 + PADDING),
                     "Velocity: (%.4f, %.4f)",
                     projectile.vel.x, projectile.vel.y);
            displayf(renderer, debug_font,
                     TRACKING_DEBUG_COLOR, vec2(PADDING + SECOND_COLUMN_OFFSET, 3 * 50 + PADDING),
                     "Shooter Index: %d",
                     projectile.shooter.unwrap);
        }

        for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
            if (entities[i].state == Entity_State::Ded) continue;

            sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
            auto dstrect = rectf_for_sdl(camera.to_screen(entities[i].texbox_world()));
            sec(SDL_RenderDrawRect(renderer, &dstrect));

            sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
            auto hitbox = rectf_for_sdl(camera.to_screen(entities[i].hitbox_world()));
            sec(SDL_RenderDrawRect(renderer, &hitbox));
        }

        if (tracking_projectile.has_value) {
            sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
            auto hitbox = rectf_for_sdl(
                camera.to_screen(hitbox_of_projectile(tracking_projectile.unwrap)));
            sec(SDL_RenderDrawRect(renderer, &hitbox));
        }

        auto projectile_index = projectile_at_position(debug_mouse_position);
        if (projectile_index.has_value) {
            sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
            auto hitbox = rectf_for_sdl(
                camera.to_screen(hitbox_of_projectile(projectile_index.unwrap)));
            sec(SDL_RenderDrawRect(renderer, &hitbox));
            return;
        }

        sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
        const Rectf tile_rect = {
            floorf(debug_mouse_position.x / TILE_SIZE) * TILE_SIZE,
            floorf(debug_mouse_position.y / TILE_SIZE) * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE
        };
        {
            auto rect = rectf_for_sdl(camera.to_screen(tile_rect));
            sec(SDL_RenderDrawRect(renderer, &rect));
        }
    }

    void init_projectiles(Frame_Animat active_animat, Frame_Animat poof_animat)
    {
        for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
            projectiles[i].active_animat = active_animat;
            projectiles[i].poof_animat = poof_animat;
        }
    }

    int count_alive_projectiles(void)
    {
        int res = 0;
        for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
            if (projectiles[i].state != Projectile_State::Ded) ++res;
        }
        return res;
    }
    void render_projectiles(SDL_Renderer *renderer, Camera camera)
    {
        for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
            switch (projectiles[i].state) {
            case Projectile_State::Active: {
                render_animat(renderer,
                              projectiles[i].active_animat,
                              camera.to_screen(projectiles[i].pos));
            } break;

            case Projectile_State::Poof: {
                render_animat(renderer,
                              projectiles[i].poof_animat,
                              camera.to_screen(projectiles[i].pos));
            } break;

            case Projectile_State::Ded: {} break;
            }
        }
    }

    void update_projectiles(float dt)
    {
        for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
            switch (projectiles[i].state) {
            case Projectile_State::Active: {
                update_animat(&projectiles[i].active_animat, dt);
                projectiles[i].pos += projectiles[i].vel * dt;

                int room_current = (int) floorf(projectiles[i].pos.x / ROOM_BOUNDARY.w);
                if (0 <= room_current && room_current < (int) ROOM_ROW_COUNT) {
                    if (!room_row[room_current].is_tile_at_abs_p_empty(projectiles[i].pos)) {
                        projectiles[i].state = Projectile_State::Poof;
                        projectiles[i].poof_animat.frame_current = 0;
                    }
                }

                projectiles[i].lifetime -= dt;

                if (projectiles[i].lifetime <= 0.0f) {
                    projectiles[i].state = Projectile_State::Poof;
                    projectiles[i].poof_animat.frame_current = 0;
                }
            } break;

            case Projectile_State::Poof: {
                update_animat(&projectiles[i].poof_animat, dt);
                if (projectiles[i].poof_animat.frame_current ==
                    (projectiles[i].poof_animat.frame_count - 1)) {
                    projectiles[i].state = Projectile_State::Ded;
                }
            } break;

            case Projectile_State::Ded: {} break;
            }
        }

    }


    Rectf hitbox_of_projectile(Projectile_Index index)
    {
        assert(index.unwrap < PROJECTILES_COUNT);
        return Rectf {
            projectiles[index.unwrap].pos.x - PROJECTILE_TRACKING_PADDING * 0.5f,
                projectiles[index.unwrap].pos.y - PROJECTILE_TRACKING_PADDING * 0.5f,
                PROJECTILE_TRACKING_PADDING,
                PROJECTILE_TRACKING_PADDING
                };
    }

    Maybe<Projectile_Index> projectile_at_position(Vec2f position)
    {
        for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
            if (projectiles[i].state == Projectile_State::Ded) continue;

            Rectf hitbox = hitbox_of_projectile({i});
            if (rect_contains_vec2(hitbox, position)) {
                return {true, {i}};
            }
        }

        return {};
    }

};


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

    stec(TTF_Init());
    const int DEBUG_FONT_SIZE = 32;

    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

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

    game_state.init_projectiles(plasma_bolt_animat, plasma_pop_animat);

    char room_file_path[256];
    for (size_t room_index = 0; room_index < ROOM_ROW_COUNT; ++room_index) {
        room_row[room_index].position = {(float) room_index * ROOM_BOUNDARY.w, 0};
        room_row[room_index].load_file(
            file_path_of_room(
                room_file_path,
                sizeof(room_file_path),
                {room_index}));
    }

    game_state.reset_entities(walking, idle, jump_sample1, jump_sample2);

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
                            game_state.entities[PLAYER_ENTITY_INDEX].jump(game_state.gravity, &mixer);
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
                        game_state.reset_entities(walking, idle, jump_sample1, jump_sample2);
                    } break;
                    }
                }
            } break;

            case SDL_KEYUP: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    if (!event.key.repeat) {
                        game_state.entities[PLAYER_ENTITY_INDEX].jump(game_state.gravity, &mixer);
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
                    game_state.entities[PLAYER_ENTITY_INDEX].vel.x =
                        fminf(
                            game_state.entities[PLAYER_ENTITY_INDEX].vel.x + PLAYER_ACCEL * SIMULATION_DELTA_TIME,
                            PLAYER_SPEED);
                    game_state.entities[PLAYER_ENTITY_INDEX].alive_state = Alive_State::Walking;
                } else if (keyboard[SDL_SCANCODE_A]) {
                    game_state.entities[PLAYER_ENTITY_INDEX].vel.x =
                        fmax(
                            game_state.entities[PLAYER_ENTITY_INDEX].vel.x - PLAYER_ACCEL * SIMULATION_DELTA_TIME,
                            -PLAYER_SPEED);
                    game_state.entities[PLAYER_ENTITY_INDEX].alive_state = Alive_State::Walking;
                } else {
                    const float PLAYER_STOP_THRESHOLD = 100.0f;
                    if (fabs(game_state.entities[PLAYER_ENTITY_INDEX].vel.x) > PLAYER_STOP_THRESHOLD) {
                        game_state.entities[PLAYER_ENTITY_INDEX].vel.x -=
                            sgn(game_state.entities[PLAYER_ENTITY_INDEX].vel.x) * PLAYER_ACCEL * SIMULATION_DELTA_TIME;
                    } else {
                        game_state.entities[PLAYER_ENTITY_INDEX].vel.x = 0.0f;
                    }
                    game_state.entities[PLAYER_ENTITY_INDEX].alive_state = Alive_State::Idle;
                }

                const float PLAYER_CAMERA_FORCE = 2.0f;
                const float CENTER_CAMERA_FORCE = PLAYER_CAMERA_FORCE * 2.0f;

                const auto player_pos = game_state.entities[PLAYER_ENTITY_INDEX].pos;
                const auto room_center = room_row[room_index_at(player_pos).unwrap].center();

                game_state.camera.vel =
                    (player_pos - game_state.camera.pos) * PLAYER_CAMERA_FORCE +
                    (room_center - game_state.camera.pos) * CENTER_CAMERA_FORCE;
                game_state.camera.update(SIMULATION_DELTA_TIME);

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
