#include "something_game.hpp"

template <typename ... Types>
void displayf(SDL_Renderer *renderer,
              Bitmap_Font *font,
              RGBA color,
              RGBA shadow_color,
              Vec2f p,
              Types... args)
{
    char text[256];
    String_Buffer sbuffer = {256, text, 0};
    sprintln(&sbuffer, args...);

    auto font_size = vec2(FONT_DEBUG_SIZE, FONT_DEBUG_SIZE);
    font->render(renderer, p - vec2(2.0f, 2.0f), font_size, shadow_color, text);
    font->render(renderer, p, font_size, color, text);
}

void Game::handle_event(SDL_Event *event)
{
    // GLOBAL KEYBINDINGS //////
    switch (event->type) {
    case SDL_QUIT: {
        quit = true;
    } break;

    case SDL_KEYDOWN: {
        switch (event->key.keysym.sym) {
        case SDLK_BACKSLASH:
        case SDLK_BACKQUOTE: {
            console.toggle();
        } break;


#ifndef SOMETHING_RELEASE
        case SDLK_F2: {
            fps_debug = !fps_debug;
        } break;

        case SDLK_F3: {
            bfs_debug = !bfs_debug;
        } break;

        case SDLK_F5: {
            command_reload(this, ""_sv);
        } break;
#endif  // SOMETHING_RELEASE
        }
    } break;

    case SDL_MOUSEMOTION: {
        if (debug) {
            debug_toolbar.handle_mouse_hover(
                vec_cast<float>(vec2(event->motion.x, event->motion.y)));
            debug_toolbar.buttons[debug_toolbar.active_button].tool.handle_event(this, event);
        }

        grid.resolve_point_collision(&collision_probe);

        auto weapon = entities[PLAYER_ENTITY_INDEX].get_current_weapon();
        if (weapon != NULL &&
            weapon->type == Weapon_Type::Placer &&
            holding_down_mouse)
        {
            entity_shoot({PLAYER_ENTITY_INDEX});
        }
    } break;

    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_RIGHT: {
            if (debug) {
                tracking_projectile =
                    projectile_at_position(mouse_position);

                if (!tracking_projectile.has_value) {
                    debug_toolbar.buttons[debug_toolbar.active_button].tool.handle_event(this, event);
                }
            }
        } break;

        case SDL_BUTTON_LEFT: {
            if (!debug_toolbar.handle_click_at({(float) event->button.x, (float) event->button.y})) {
                entity_shoot({PLAYER_ENTITY_INDEX});
            }

            holding_down_mouse = true;
        } break;
        }
    } break;

    case SDL_MOUSEBUTTONUP: {
        switch (event->button.button) {
        case SDL_BUTTON_LEFT: {
            holding_down_mouse = false;
        } break;
        }

        if (debug) {
            debug_toolbar.buttons[debug_toolbar.active_button].tool.handle_event(this, event);
        }
    } break;
    }

    if (console.enabled) {
        console.handle_event(event, this);
    } else {
        switch (event->type) {
        case SDL_KEYDOWN: {
            switch (event->key.keysym.sym) {
            case SDLK_SPACE: {
                if (!event->key.repeat) {
                    if(entities[PLAYER_ENTITY_INDEX].noclip) {
                        entities[PLAYER_ENTITY_INDEX].vel.y =
                            -entities[PLAYER_ENTITY_INDEX].speed;
                    } else {
                        entity_jump({PLAYER_ENTITY_INDEX});
                    }
                }
            } break;

            case SDLK_w: {
                if (debug) {
                    entities[PLAYER_ENTITY_INDEX].vel.y = -entities[PLAYER_ENTITY_INDEX].speed;
                }
            } break;

            case SDLK_s: {
                if (debug) {
                    entities[PLAYER_ENTITY_INDEX].vel.y = entities[PLAYER_ENTITY_INDEX].speed;
                }
            } break;

            case SDLK_n: {
                noclip(!entities[PLAYER_ENTITY_INDEX].noclip);
            } break;

            case SDLK_q: {
                debug = !debug;
                if (debug) {
                    for (size_t i = ENEMY_ENTITY_INDEX_OFFSET; i < ENTITIES_COUNT; ++i) {
                        if (entities[i].state == Entity_State::Alive) {
                            entities[i].stop();
                        }
                    }
                } else {
                    entities[PLAYER_ENTITY_INDEX].noclip = false;
                }
            } break;

            case SDLK_z: {
                step_debug = !step_debug;
            } break;

            case SDLK_r: {
                reset_entities();
            } break;
            }
        } break;

        case SDL_KEYUP: {
            if (entities[PLAYER_ENTITY_INDEX].noclip) {
                switch (event->key.keysym.sym) {
                case SDLK_SPACE:
                case SDLK_w:
                case SDLK_s: {
                    if (debug) {
                        entities[PLAYER_ENTITY_INDEX].vel.y = 0.0f;
                    }
                } break;
                }
            }
        } break;

        case SDL_MOUSEWHEEL: {
            if (event->wheel.y < 0) {
                entities[PLAYER_ENTITY_INDEX].weapon_current =
                    mod((int) entities[PLAYER_ENTITY_INDEX].weapon_current + 1,
                        (int) entities[PLAYER_ENTITY_INDEX].weapon_slots_count);
            } else if (event->wheel.y > 0) {
                entities[PLAYER_ENTITY_INDEX].weapon_current =
                    mod((int) entities[PLAYER_ENTITY_INDEX].weapon_current - 1,
                        (int) entities[PLAYER_ENTITY_INDEX].weapon_slots_count);
            }
        } break;
        }
    }
}

void Game::update(float dt)
{
    entities[PLAYER_ENTITY_INDEX].point_gun_at(mouse_position);

    for (size_t i = 0; i < SPIKES_COUNT; ++i) {
        spikes[i].update(dt);
    }

    for (size_t i = 0; i < SPIKE_WAVES_CAPACITY; ++i) {
        spike_waves[i].update(dt, this);
    }

    // Enemy AI //////////////////////////////
    auto &player = entities[PLAYER_ENTITY_INDEX];
    Recti *lock = NULL;
    for (size_t i = 0; i < camera_locks_count; ++i) {
        Rectf lock_abs = rect_cast<float>(camera_locks[i]) * TILE_SIZE;
        if (rect_contains_vec2(lock_abs, player.pos)) {
            lock = &camera_locks[i];
        }
    }
    player.get_current_weapon()->update(dt);

    auto player_tile = grid.abs_to_tile_coord(player.pos);
    if (lock) {
        grid.bfs_to_tile(player_tile, lock);
    }

    if (!debug && lock) {
        Rectf lock_abs = rect_cast<float>(*lock) * TILE_SIZE;
        for (size_t i = ENEMY_ENTITY_INDEX_OFFSET; i < ENTITIES_COUNT; ++i) {
            auto &enemy =  entities[i];
            if (enemy.state == Entity_State::Alive) {
                if (rect_contains_vec2(lock_abs, enemy.pos)) {
                    if (enemy.brain.think) {
                        enemy.brain.think(this, {i}, lock);
                    }
                }
            }
        }
    }

    // Update All Entities //////////////////////////////
    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        entities[i].update(dt, this, {i});
        if (!entities[i].noclip) entity_resolve_collision({i});
        entities[i].has_jumped = false;

        if (entities[i].pos.y > WORLD_DEATH_LIMIT) {
            kill_entity(&entities[i]);
        }
    }

    // Update All Projectiles //////////////////////////////
    update_projectiles(dt);

    // Update Items //////////////////////////////
    for (size_t i = 0; i < ITEMS_COUNT; ++i) {
        items[i].update(dt);
    }

    // Entities/Projectiles interaction //////////////////////////////
    for (size_t index = 0; index < PROJECTILES_COUNT; ++index) {
        auto projectile = projectiles + index;
        if (projectile->state != Projectile_State::Active) continue;

        for (size_t entity_index = 0;
             entity_index < ENTITIES_COUNT;
             ++entity_index)
        {
            auto entity = entities + entity_index;

            if (entity->state == Entity_State::Alive) {
                if (entity_index != projectile->shooter.unwrap) {
                    if (rect_contains_vec2(entity->hitbox_world(), projectile->pos)) {
                        projectile->kill();
                        damage_entity(
                            entity,
                            ENTITY_PROJECTILE_DAMAGE,
                            normalize(projectile->vel) * ENTITY_PROJECTILE_KNOCKBACK);
                    }
                }
            }
        }
    }

    // Entities/Items interaction
    for (size_t index = 0; index < ITEMS_COUNT; ++index) {
        auto item = items + index;
        if (item->type != ITEM_NONE) {
            for (size_t entity_index = 0;
                 entity_index < ENTITIES_COUNT;
                 ++entity_index)
            {
                auto entity = entities + entity_index;

                if (entity->state == Entity_State::Alive) {
                    if (rects_overlap(entity->hitbox_world(), item->hitbox_world())) {
                        switch (item->type) {
                        case ITEM_NONE: {
                            assert(0 && "unreachable");
                        } break;

                        case ITEM_HEALTH: {
                            entity->lives = min(entity->lives + ITEM_HEALTH_POINTS, ENTITY_MAX_LIVES);
                            entity->flash(ENTITY_HEAL_FLASH_COLOR);
                            mixer.play_sample(POP_SOUND_INDEX);
                            item->type = ITEM_NONE;
                        } break;

                        case ITEM_DIRT_BLOCK: {
                            for (size_t i = 0; i < entity->weapon_slots_count; ++i) {
                                if (entity->weapon_slots[i].type == Weapon_Type::Placer &&
                                    entity->weapon_slots[i].placer.tile == TILE_DIRT_0)
                                {
                                    entity->weapon_slots[i].placer.amount += 1;
                                    break;
                                }
                            }
                            mixer.play_sample(POP_SOUND_INDEX);
                            item->type = ITEM_NONE;
                        } break;

                        case ITEM_ICE_BLOCK: {
                            for (size_t i = 0; i < entity->weapon_slots_count; ++i) {
                                if (entity->weapon_slots[i].type == Weapon_Type::Placer &&
                                    entity->weapon_slots[i].placer.tile == TILE_ICE_0)
                                {
                                    entity->weapon_slots[i].placer.amount += 1;
                                    break;
                                }
                            }
                            mixer.play_sample(POP_SOUND_INDEX);
                            item->type = ITEM_NONE;
                        } break;
                        }
                        break;
                    }
                }
            }
        }
    }

    //  Projectiles/Projectiles Interaction /////////
    for (size_t i = 0; i < PROJECTILES_COUNT - 1; ++i) {
        for (size_t j = i + 1; j < PROJECTILES_COUNT; ++j) {
            projectile_collision(&projectiles[i], &projectiles[j]);
        }
    }

    // Player Movement //////////////////////////////
    if (!console.enabled) {
        if (keyboard[SDL_SCANCODE_D]) {
            entities[PLAYER_ENTITY_INDEX].move(Entity::Right);
        } else if (keyboard[SDL_SCANCODE_A]) {
            entities[PLAYER_ENTITY_INDEX].move(Entity::Left);
        } else {
            entities[PLAYER_ENTITY_INDEX].stop();
        }
    }

    // Camera "Physics" //////////////////////////////
    const auto player_pos = entities[PLAYER_ENTITY_INDEX].pos;
    if(entities[PLAYER_ENTITY_INDEX].noclip) {
        camera.vel = (player_pos - camera.pos) * NOCLIP_CAMERA_FORCE;
    } else {
        camera.vel = (player_pos - camera.pos) * PLAYER_CAMERA_FORCE;

        for (size_t i = 0; i < camera_locks_count; ++i) {
            Rectf lock_abs = rect_cast<float>(camera_locks[i]) * TILE_SIZE;
            if (rect_contains_vec2(lock_abs, player_pos)) {
                camera.vel += (rect_center(lock_abs) - camera.pos) * CENTER_CAMERA_FORCE;
            }
        }

        if (camera_shaking_timeout > 0.0f) {
            auto random_shaking_vector = polar(CAMERA_SHAKING_INTENSITY, rand_float_range(0.0f, 2.0f * PI));
            camera.vel += random_shaking_vector;
            camera_shaking_timeout -= dt;
        }
    }

    camera.update(dt);

    // Popup //////////////////////////////
    popup.update(dt);

    // Console //////////////////////////////
    console.update(dt);
}

void Game::render(SDL_Renderer *renderer)
{
    Recti *lock = NULL;
    for (size_t i = 0; i < camera_locks_count; ++i) {
        Rectf lock_abs = rect_cast<float>(camera_locks[i]) * TILE_SIZE;
        if (rect_contains_vec2(lock_abs, entities[PLAYER_ENTITY_INDEX].pos)) {
            lock = &camera_locks[i];
        }
    }

    background.render(renderer, camera);

    for (size_t i = 0; i < SPIKES_COUNT; ++i) {
        spikes[i].render(renderer, camera);
    }

    if (bfs_debug && lock) {
        grid.render_debug_bfs_overlay(
            renderer,
            &camera,
            lock);
    }

    grid.render(renderer, camera, lock);

    entities[PLAYER_ENTITY_INDEX].render(renderer, camera);
    if (lock) {
        Rectf lock_abs = rect_cast<float>(*lock) * TILE_SIZE;
        for (size_t i = ENEMY_ENTITY_INDEX_OFFSET; i < ENTITIES_COUNT; ++i) {
            if (rect_contains_vec2(lock_abs, entities[i].pos)) {
                entities[i].render(renderer, camera);
            } else {
                entities[i].render(renderer, camera, ROOM_NEIGHBOR_DIM_COLOR);
            }
        }
    } else {
        for (size_t i = ENEMY_ENTITY_INDEX_OFFSET; i < ENTITIES_COUNT; ++i) {
            entities[i].render(renderer, camera, ROOM_NEIGHBOR_DIM_COLOR);
        }
    }

    // TODO(#353): enemies don't have their weapons renderered
    {
        auto weapon = entities[PLAYER_ENTITY_INDEX].get_current_weapon();
        if (weapon != NULL) {
            weapon->render(renderer, this, {PLAYER_ENTITY_INDEX});
        }
    }

    render_projectiles(renderer, camera);

    for (size_t i = 0; i < ITEMS_COUNT; ++i) {
        if (items[i].type != ITEM_NONE) {
            items[i].render(renderer, camera);
        }
    }

    if (fps_debug) {
        render_fps_overlay(renderer);
    }

    render_player_hud(renderer);

    popup.render(renderer);
    console.render(renderer, &debug_font);
}

void Game::entity_shoot(Index<Entity> entity_index)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);
    Entity *entity = &entities[entity_index.unwrap];

    if (entity->state == Entity_State::Alive) {
        auto weapon = entity->get_current_weapon();

        if (weapon != NULL) {
            switch (weapon->type) {
            case Weapon_Type::Gun: {
                if (entity->cooldown_weapon <= 0) {
                    weapon->shoot(this, entity_index);
                    if (weapon->shoot_sample.has_value) {
                        mixer.play_sample(weapon->shoot_sample.unwrap);
                    }

                    // TODO(#305): can we move cooldown_weapon to the Weapon struct
                    entity->cooldown_weapon = ENTITY_COOLDOWN_WEAPON;
                }
            } break;

            case Weapon_Type::Stomp:
            case Weapon_Type::Placer: {
                weapon->shoot(this, entity_index);
            } break;
            }
        }
    }
}

bool Game::does_tile_contain_entity(Vec2i tile_coord)
{
    Rectf tile_rect = grid.rect_of_tile(tile_coord);

    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        if (entities[i].state == Entity_State::Alive && rects_overlap(tile_rect, entities[i].hitbox_world())) {
            return true;
        }
    }

    return false;
}

Vec2i Game::where_entity_can_place_block(Index<Entity> index, bool *can_place)
{
    Entity *entity = &entities[index.unwrap];
    const auto allowed_length = min(length(entity->gun_dir), DIRT_BLOCK_PLACEMENT_PROXIMITY);
    const auto allowed_target = entity->pos + allowed_length *normalize(entity->gun_dir);
    const auto target_tile = grid.abs_to_tile_coord(allowed_target);

    if (can_place) {
        *can_place = grid.get_tile(target_tile) == TILE_EMPTY &&
            grid.a_sees_b(entity->pos, grid.abs_center_of_tile(target_tile)) &&
            !does_tile_contain_entity(target_tile);
    }

    return target_tile;
}


void Game::entity_jump(Index<Entity> entity_index)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);
    entities[entity_index.unwrap].jump();
}

void Game::reset_entities()
{
    static_assert(ROOM_ROW_COUNT > 0);
    entities[PLAYER_ENTITY_INDEX] = player_entity(vec2(200.0f, 200.0f));
}

void Game::entity_resolve_collision(Index<Entity> entity_index)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);
    Entity *entity = &entities[entity_index.unwrap];

    if (entity->state == Entity_State::Alive) {
        const float step_x =
            entity->hitbox_local.w / ceilf(entity->hitbox_local.w / ENTITY_MESH_STEP);
        const float step_y =
            entity->hitbox_local.h / ceilf(entity->hitbox_local.h / ENTITY_MESH_STEP);

        const Vec2f origin = entity->pos;

        for (int rows = 0; rows * step_x <= entity->hitbox_local.w; ++rows) {
            for (int cols = 0; cols * step_y <= entity->hitbox_local.h; ++cols) {
                Vec2f t0 = entity->pos +
                    vec2(entity->hitbox_local.x, entity->hitbox_local.y) +
                    vec2(cols * step_x, rows * step_y);
                Vec2f t1 = t0;
                grid.resolve_point_collision(&t1);
                entity->pos += t1 - t0;
            }
        }

        const Vec2f d = entity->pos - origin;

        // NOTE: Don't consider y-impact if x-impact has happened
        // This is needed to not get stuck in the wall in the middle of the air
        if (abs(d.x) >= ENTITY_IMPACT_THRESHOLD) {
            entity->vel.x = 0;
        } else if (abs(d.y) >= ENTITY_IMPACT_THRESHOLD && !entity->has_jumped) {
            if (fabsf(entity->vel.y) > LANDING_PARTICLE_BURST_THRESHOLD) {
                for (int i = 0; i < ENTITY_JUMP_PARTICLE_BURST; ++i) {
                    entity->particles.push(rand_float_range(PARTICLE_JUMP_VEL_LOW, fabsf(entity->vel.y) * 0.25f));
                }
            }

            entity->vel.y = 0;
        }
    }
}

void Game::spawn_projectile(Projectile projectile)
{
    for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
        if (projectiles[i].state == Projectile_State::Ded) {
            projectiles[i] = projectile;
            return;
        }
    }
}

void Game::render_debug_overlay(SDL_Renderer *renderer, size_t fps)
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

    const float PADDING = 10.0f;
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, PADDING),
             "FPS: ", fps);
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, 50 + PADDING),
             "Mouse Position: ",
             mouse_position.x, " ",
             mouse_position.y);
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, 2 * 50 + PADDING),
             "Collision Probe: ",
             collision_probe.x, " ",
             collision_probe.y);
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, 3 * 50 + PADDING),
             "Projectiles: ",
             count_alive_projectiles());
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, 4 * 50 + PADDING),
             "Player position: ",
             entities[PLAYER_ENTITY_INDEX].pos.x, " ",
             entities[PLAYER_ENTITY_INDEX].pos.y);
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, 5 * 50 + PADDING),
             "Player velocity: ",
             entities[PLAYER_ENTITY_INDEX].vel.x, " ",
             entities[PLAYER_ENTITY_INDEX].vel.y);

    if (tracking_projectile.has_value) {
        auto projectile = projectiles[tracking_projectile.unwrap.unwrap];
        const float SECOND_COLUMN_OFFSET = 700.0f;
        const RGBA TRACKING_DEBUG_COLOR = sdl_to_rgba({255, 255, 150, 255});
        displayf(renderer, &debug_font,
                 TRACKING_DEBUG_COLOR,
                 FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, PADDING),
                 "State: ", projectile_state_as_cstr(projectile.state));
        displayf(renderer, &debug_font,
                 TRACKING_DEBUG_COLOR,
                 FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, 50 + PADDING),
                 "Position: ",
                 projectile.pos.x, " ", projectile.pos.y);
        displayf(renderer, &debug_font,
                 TRACKING_DEBUG_COLOR,
                 FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, 2 * 50 + PADDING),
                 "Velocity: ",
                 projectile.vel.x, " ", projectile.vel.y);
        displayf(renderer, &debug_font,
                 TRACKING_DEBUG_COLOR,
                 FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, 3 * 50 + PADDING),
                 "Shooter Index: ",
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

        entities[i].render_debug(renderer, camera, &debug_font);
    }

    for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
        if (projectiles[i].state == Projectile_State::Active) {
            draw_rect(renderer, camera.to_screen(projectiles[i].hitbox()), RGBA_RED);
        }

    }

    if (tracking_projectile.has_value) {
        sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
        auto hitbox = rectf_for_sdl(
            camera.to_screen(hitbox_of_projectile(tracking_projectile.unwrap)));
        sec(SDL_RenderDrawRect(renderer, &hitbox));
    }

    auto projectile_index = projectile_at_position(mouse_position);
    if (projectile_index.has_value) {
        sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
        auto hitbox = rectf_for_sdl(
            camera.to_screen(hitbox_of_projectile(projectile_index.unwrap)));
        sec(SDL_RenderDrawRect(renderer, &hitbox));
    } else {
        sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));
        const Rectf tile_rect = {
            floorf(mouse_position.x / TILE_SIZE) * TILE_SIZE,
            floorf(mouse_position.y / TILE_SIZE) * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE
        };

        auto rect = rectf_for_sdl(camera.to_screen(tile_rect));
        sec(SDL_RenderDrawRect(renderer, &rect));
    }

    for (size_t i = 0; i < ITEMS_COUNT; ++i) {
        items[i].render_debug(renderer, camera);
    }

    debug_toolbar.render(renderer, debug_font);
}

void Game::render_fps_overlay(SDL_Renderer *renderer) {
    const float PADDING = 20.0f;
    const float SCALE = 5000.0f;
    const float BAR_WIDTH = 2.0f;
    for (size_t i = 0; i < FPS_BARS_COUNT; ++i) {
        size_t j = (frame_delays_begin + i) % FPS_BARS_COUNT;
        fill_rect(
            renderer,
            rect(
                vec2(SCREEN_WIDTH - PADDING - (float) (FPS_BARS_COUNT - j) * BAR_WIDTH,
                    SCREEN_HEIGHT - PADDING - frame_delays[j] * SCALE),
                BAR_WIDTH,
                frame_delays[j] * SCALE),
                {   clamp(       frame_delays[j] * 60.0f - 1.0f, 0.0f, 1.0f),
                    clamp(2.0f - frame_delays[j] * 60.0f       , 0.0f, 1.0f),
                    0, (float) i / (float) FPS_BARS_COUNT});
    }
}

int Game::count_alive_projectiles(void)
{
    int res = 0;
    for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
        if (projectiles[i].state != Projectile_State::Ded) ++res;
    }
    return res;
}

void Game::render_projectiles(SDL_Renderer *renderer, Camera camera)
{
    for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
        projectiles[i].render(renderer, &camera);
    }
}

void Game::update_projectiles(float dt)
{
    for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
        projectiles[i].update(dt, &grid);
    }
}

const float PROJECTILE_TRACKING_PADDING = 50.0f;

Rectf Game::hitbox_of_projectile(Index<Projectile> index)
{
    assert(index.unwrap < PROJECTILES_COUNT);

    return Rectf {
        projectiles[index.unwrap].pos.x - PROJECTILE_TRACKING_PADDING * 0.5f,
            projectiles[index.unwrap].pos.y - PROJECTILE_TRACKING_PADDING * 0.5f,
            PROJECTILE_TRACKING_PADDING,
            PROJECTILE_TRACKING_PADDING
    };
}

Maybe<Index<Projectile>> Game::projectile_at_position(Vec2f position)
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

void Game::spawn_dirt_block_item_at(Vec2f pos)
{
    for (size_t i = 0; i < ITEMS_COUNT; ++i) {
        if (items[i].type == ITEM_NONE) {
            items[i] = make_dirt_block_item(pos);
            break;
        }
    }
}

void Game::spawn_dirt_block_item_at_mouse()
{
    spawn_dirt_block_item_at(mouse_position);
}

void Game::spawn_item_at(Item item, Vec2f pos)
{
    item.pos = pos;
    for (size_t i = 0; i < ITEMS_COUNT; ++i) {
        if (items[i].type == ITEM_NONE) {
            items[i] = item;
            break;
        }
    }
}

void Game::spawn_health_at_mouse()
{
    for (size_t i = 0; i < ITEMS_COUNT; ++i) {
        if (items[i].type == ITEM_NONE) {
            items[i] = make_health_item(mouse_position);
            break;
        }
    }
}

void Game::add_camera_lock(Recti rect)
{
    assert(camera_locks_count < CAMERA_LOCKS_CAPACITY);
    camera_locks[camera_locks_count++] = rect;
}

void Game::spawn_entity_at(Entity entity, Vec2f pos)
{
    entity.pos = pos;
    for (size_t i = PLAYER_ENTITY_INDEX + ENEMY_ENTITY_INDEX_OFFSET; i < ENTITIES_COUNT; ++i) {
        if (entities[i].state == Entity_State::Ded) {
            entities[i] = entity;
            break;
        }
    }
}

void Game::spawn_enemy_at(Vec2f pos)
{
    for (size_t i = PLAYER_ENTITY_INDEX + ENEMY_ENTITY_INDEX_OFFSET; i < ENTITIES_COUNT; ++i) {
        if (entities[i].state == Entity_State::Ded) {
            entities[i] = enemy_entity(pos);
            break;
        }
    }
}

void Game::spawn_golem_at(Vec2f pos)
{
    for (size_t i = PLAYER_ENTITY_INDEX + ENEMY_ENTITY_INDEX_OFFSET; i < ENTITIES_COUNT; ++i) {
        if (entities[i].state == Entity_State::Ded) {
            entities[i] = golem_entity(pos);
            break;
        }
    }
}

int Game::get_rooms_count(void)
{
    int result = 0;
    DIR *rooms_dir = opendir("./assets/rooms/");
    if (rooms_dir == NULL) {
        println(stderr, "Can't open asset folder: ./assets/rooms/");
        abort();
    }

    for (struct dirent *d = readdir(rooms_dir);
        d != NULL;
        d = readdir(rooms_dir)) {
        if (*d->d_name == '.') continue;
        result++;
    }

    closedir(rooms_dir);
    return result;
}

void Game::render_player_hud(SDL_Renderer *renderer)
{
    Entity *player = &entities[PLAYER_ENTITY_INDEX];

    const size_t MAXIMUM_LENGTH = 3;
    char label[MAXIMUM_LENGTH + 1];

    auto text_width = MAXIMUM_LENGTH * BITMAP_FONT_CHAR_WIDTH * PLAYER_HUD_FONT_SIZE;
    auto text_height = BITMAP_FONT_CHAR_HEIGHT * PLAYER_HUD_FONT_SIZE;

    Vec2f border_size = vec2(
        PLAYER_HUD_ICON_WIDTH + PADDING_BETWEEN_TEXT_AND_ICON + text_width + PLAYER_HUD_PADDING * 2,
        max(PLAYER_HUD_ICON_HEIGHT, text_height) + PLAYER_HUD_PADDING * 2);

    for (size_t i = 0; i < player->weapon_slots_count; ++i) {
        const auto position = vec2(PLAYER_HUD_MARGIN, PLAYER_HUD_MARGIN + (border_size.y + PLAYER_HUD_MARGIN) * i);
        fill_rect(renderer, rect(position, border_size), i == player->weapon_current ? PLAYER_HUD_SELECTED_COLOR : PLAYER_HUD_BACKGROUND_COLOR);
        Rectf destrect = rect(position + vec2(PLAYER_HUD_PADDING, PLAYER_HUD_PADDING),
                              vec2(PLAYER_HUD_ICON_WIDTH, PLAYER_HUD_ICON_HEIGHT));

        player->weapon_slots[i].icon().render(renderer, destrect);

        switch (player->weapon_slots[i].type) {
        case Weapon_Type::Stomp:
        case Weapon_Type::Gun:
            snprintf(label, sizeof(label), "inf");
            break;

        case Weapon_Type::Placer:
            snprintf(label, sizeof(label), "%d", (unsigned) player->weapon_slots[i].placer.amount);
            break;
        }

        debug_font.render(
            renderer,
            position + vec2(PLAYER_HUD_PADDING + PLAYER_HUD_ICON_WIDTH + PADDING_BETWEEN_TEXT_AND_ICON, PLAYER_HUD_PADDING),
            vec2(PLAYER_HUD_FONT_SIZE, PLAYER_HUD_FONT_SIZE),
            PLAYER_HUD_FONT_COLOR,
            label);
    }
}

void Game::noclip(bool on)
{
    if (debug) {
        entities[PLAYER_ENTITY_INDEX].noclip = on;
        if (entities[PLAYER_ENTITY_INDEX].noclip) {
            popup.notify(FONT_SUCCESS_COLOR, "Noclip enabled");
            entities[PLAYER_ENTITY_INDEX].vel.y = 0;
        } else {
            popup.notify(FONT_FAILURE_COLOR, "Noclip disabled");
        }
    }
}

void Game::projectile_collision(Projectile *a, Projectile *b)
{
    if (a != b &&
        a->state == Projectile_State::Active &&
        b->state == Projectile_State::Active)
    {
        if (rects_overlap(a->hitbox(), b->hitbox())) {
            if ((a->kind == Projectile_Kind::Ice && b->kind == Projectile_Kind::Fire) ||
                (a->kind == Projectile_Kind::Rock && b->kind == Projectile_Kind::Water) ||
                (a->kind == Projectile_Kind::Fire && b->kind == Projectile_Kind::Water))
            {
                swap(&a, &b);
            }

            if (a->kind == Projectile_Kind::Fire && b->kind == Projectile_Kind::Ice) {
                spawn_projectile(
                    water_projectile(
                        b->pos,
                        normalize(a->vel + b->vel) * PROJECTILE_SPEED,
                        b->shooter));
                a->kill();
                b->kill();
            }

            if (a->kind == Projectile_Kind::Water && b->kind == Projectile_Kind::Rock) {
                a->kill();
                b->kill();
            }

            if (a->kind == Projectile_Kind::Water && b->kind == Projectile_Kind::Fire) {
                b->kill();
            }
        }
    }
}

void Game::drop_all_items_of_entity(Entity *entity)
{
    for (size_t i = 0; i < entity->items_count; ++i) {
        for (size_t j = 0; j < entity->items[i].count; ++j) {
            spawn_item_at(entity->items[i].item, entity->pos);
        }
    }

    entity->items_count = 0;
}

void Game::kill_entity(Entity *entity)
{
    if (entity->state == Entity_State::Alive) {
        entity->state = Entity_State::Poof;
        drop_all_items_of_entity(entity);
    }
}

void Game::damage_entity(Entity *entity, int amount, Vec2f knockback)
{
    entity->lives -= amount;

    mixer.play_sample(OOF_SOUND_INDEX);
    if (entity->lives <= 0) {
        kill_entity(entity);
        mixer.play_sample(CRUNCH_SOUND_INDEX);
    } else {
        entity->vel += knockback;
        entity->flash(ENTITY_DAMAGE_FLASH_COLOR);
    }
}

void Game::damage_radius(Vec2f center, float radius, Index<Entity> stomper)
{
    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        if (i != stomper.unwrap && entities[i].state == Entity_State::Alive) {
            Vec2f dir = entities[i].pos - center;
            if (sqr_len(dir) <= radius * radius) {
                damage_entity(
                    &entities[i],
                    ENTITY_STOMP_DAMAGE,
                    normalize(dir) * ENTITY_STOMP_KNOCKBACK);
            }
        }
    }
}

void Game::shake_camera(float duration)
{
    camera_shaking_timeout = duration;
}

void Game::spawn_spike(Spike spike)
{
    for (size_t i = 0; i < SPIKES_COUNT; ++i) {
        if (spikes[i].state == Spike_State::Ded) {
            spikes[i] = spike;
            spikes[i].activate();
            return;
        }
    }
}

void Game::spawn_spike_wave(Vec2f pos, Vec2f dir)
{
    for (size_t i = 0; i < SPIKE_WAVES_CAPACITY; ++i) {
        if (spike_waves[i].count == 0) {
            spike_waves[i].activate(pos, dir);
            return;
        }
    }
}
