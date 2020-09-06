#include "something_game.hpp"

const char *projectile_state_as_cstr(Projectile_State state)
{
    switch (state) {
    case Projectile_State::Ded: return "Ded";
    case Projectile_State::Active: return "Active";
    case Projectile_State::Poof: return "Poof";
    }

    assert(0 && "Incorrect Projectile_State");
}

// TODO(#25): Turn displayf into println style
void displayf(SDL_Renderer *renderer,
              Bitmap_Font *font,
              SDL_Color color,
              SDL_Color shadow_color,
              Vec2f p,
              const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char text[256];
    vsnprintf(text, sizeof(text), format, args);

    auto font_size = vec2(FONT_DEBUG_SIZE, FONT_DEBUG_SIZE);
    font->render(renderer, p - vec2(2.0f, 2.0f), font_size, shadow_color, text);
    font->render(renderer, p, font_size, color, text);

    va_end(args);
}

void Projectile::kill()
{
    if (state == Projectile_State::Active) {
        state = Projectile_State::Poof;
        poof_animat.reset();
    }
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
        case SDLK_BACKQUOTE: {
            console.toggle();
        } break;


#ifndef SOMETHING_RELEASE
        case SDLK_F5: {
            command_reload(this, ""_sv);
        } break;
#endif  // SOMETHING_RELEASE
        }
    } break;

    case SDL_MOUSEMOTION: {
        mouse_position =
            camera.to_world(vec_cast<float>(vec2(event->motion.x, event->motion.y)));
        original_mouse_position = vec2(event->motion.x, event->motion.y);
        collision_probe = mouse_position;

        if (debug) {
            debug_toolbar.handle_mouse_hover(
                vec_cast<float>(vec2(event->motion.x, event->motion.y)));
        }

        grid.resolve_point_collision(&collision_probe);

        Vec2i tile = vec_cast<int>(mouse_position / TILE_SIZE);
        switch (state) {
        case Debug_Draw_State::Create: {
            grid.set_tile(tile, TILE_DESTROYABLE_0);
        } break;

        case Debug_Draw_State::Delete: {
            grid.set_tile(tile, TILE_EMPTY);
        } break;

        case Debug_Draw_State::Idle:
        default: {}
        }
    } break;

    case SDL_MOUSEBUTTONDOWN: {
        switch (event->button.button) {
        case SDL_BUTTON_RIGHT: {
            if (debug) {
                tracking_projectile =
                    projectile_at_position(mouse_position);

                if (!tracking_projectile.has_value) {
                    switch (debug_toolbar.active_button) {
                    case DEBUG_TOOLBAR_TILES: {
                        Vec2i tile = vec_cast<int>(mouse_position / TILE_SIZE);

                        if (grid.get_tile(tile) == TILE_EMPTY) {
                            state = Debug_Draw_State::Create;
                            grid.set_tile(tile, TILE_WALL);
                        } else {
                            state = Debug_Draw_State::Delete;
                            grid.set_tile(tile, TILE_EMPTY);
                        }
                    } break;

                    case DEBUG_TOOLBAR_HEALS: {
                        spawn_health_at_mouse();
                    } break;

                    case DEBUG_TOOLBAR_ENEMIES: {
                        spawn_enemy_at(mouse_position);
                    } break;

                    default: {}
                    }
                }
            }
        } break;

        case SDL_BUTTON_LEFT: {
            if (!debug_toolbar.handle_click_at({(float)event->button.x, (float)event->button.y})) {
                entity_shoot({PLAYER_ENTITY_INDEX});
            }
        } break;
        }
    } break;

    case SDL_MOUSEBUTTONUP: {
        switch (event->button.button) {
        case SDL_BUTTON_RIGHT: {
            state = Debug_Draw_State::Idle;
        } break;
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
                    entity_jump({PLAYER_ENTITY_INDEX});
                }
            } break;

            case SDLK_q: {
                debug = !debug;
            } break;

            case SDLK_z: {
                step_debug = !step_debug;
            } break;

            case SDLK_r: {
                reset_entities();
            } break;
            }
        } break;
        }
    }
}

void Game::update(float dt)
{
    // Update Player's gun direction //////////////////////////////
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    entities[PLAYER_ENTITY_INDEX].point_gun_at(mouse_position);

    // Enemy AI //////////////////////////////
    auto &player = entities[PLAYER_ENTITY_INDEX];
    Recti *lock = NULL;
    for (size_t i = 0; i < camera_locks_count; ++i) {
        Rectf lock_abs = rect_cast<float>(camera_locks[i]) * TILE_SIZE;
        if (rect_contains_vec2(lock_abs, player.pos)) {
            lock = &camera_locks[i];
        }
    }

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
                    if (grid.a_sees_b(enemy.pos, player.pos)) {
                        enemy.stop();
                        enemy.point_gun_at(player.pos);
                        entity_shoot({i});
                    } else {
                        auto enemy_tile = grid.abs_to_tile_coord(enemy.pos);
                        auto next = grid.next_in_bfs(enemy_tile, lock);
                        if (next.has_value) {
                            auto d = next.unwrap - enemy_tile;

                            if (d.y < 0) {
                                enemy.jump();
                            }
                            if (d.x > 0) {
                                enemy.move(Entity::Right);
                            }
                            if (d.x < 0) {
                                enemy.move(Entity::Left);
                            }
                            if (d.x == 0) {
                                enemy.stop();
                            }
                        } else {
                            enemy.stop();
                        }
                    }
                }
            }
        }
    }

    // Update All Entities //////////////////////////////
    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        entities[i].update(dt, &mixer, &grid);
        entity_resolve_collision({i});
        entities[i].has_jumped = false;
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

            if (entity->state != Entity_State::Alive) continue;
            if (entity_index == projectile->shooter.unwrap) continue;

            if (rect_contains_vec2(entity->hitbox_world(), projectile->pos)) {
                projectile->kill();
                entity->lives -= ENTITY_PROJECTILE_DAMAGE;

                mixer.play_sample(damage_enemy_sample);
                if (entity->lives <= 0) {
                    entity->kill();
                    mixer.play_sample(kill_enemy_sample);
                } else {
                    entity->vel += normalize(projectile->vel) * ENTITY_PROJECTILE_KNOCKBACK;
                    entity->flash(ENTITY_DAMAGE_FLASH_COLOR);
                }
            }
        }
    }

    // Entities/Items interaction
    for (size_t index = 0; index < ITEMS_COUNT; ++index) {
        auto item = items + index;
        if (item->type == ITEM_HEALTH) {
            for (size_t entity_index = 0;
                 entity_index < ENTITIES_COUNT;
                 ++entity_index)
            {
                auto entity = entities + entity_index;

                if (entity->state == Entity_State::Alive) {
                    if (rects_overlap(entity->hitbox_world(), item->hitbox_world())) {
                        entity->lives = min(entity->lives + ITEM_HEALTH_POINTS, ENTITY_MAX_LIVES);
                        entity->flash(ENTITY_HEAL_FLASH_COLOR);
                        mixer.play_sample(item->sound);
                        item->type = ITEM_NONE;
                        break;
                    }
                }
            }
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
    camera.vel = (player_pos - camera.pos) * PLAYER_CAMERA_FORCE;

    for (size_t i = 0; i < camera_locks_count; ++i) {
        Rectf lock_abs = rect_cast<float>(camera_locks[i]) * TILE_SIZE;
        if (rect_contains_vec2(lock_abs, player_pos)) {
            camera.vel += (rect_center(lock_abs) - camera.pos) * CENTER_CAMERA_FORCE;
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

    if (debug && lock) {
        grid.render_debug_bfs_overlay(
            renderer,
            &camera,
            lock);
    }

    grid.render(renderer, camera, lock);

    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        // TODO(#106): display health bar differently for enemies in a different room
        entities[i].render(renderer, camera);
    }

    render_projectiles(renderer, camera);

    for (size_t i = 0; i < ITEMS_COUNT; ++i) {
        if (items[i].type != ITEM_NONE) {
            items[i].render(renderer, camera);
        }
    }

    popup.render(renderer);
    console.render(renderer, &debug_font);
}

void Game::entity_shoot(Entity_Index entity_index)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);
    Entity *entity = &entities[entity_index.unwrap];

    if (entity->state != Entity_State::Alive) return;
    if (entity->cooldown_weapon > 0) return;

    const float PROJECTILE_SPEED = 1200.0f;

    spawn_projectile(
        entity->pos,
        entity->gun_dir * PROJECTILE_SPEED,
        entity_index);
    entity->cooldown_weapon = ENTITY_COOLDOWN_WEAPON;

    mixer.play_sample(entity->shoot_sample);
}

void Game::entity_jump(Entity_Index entity_index)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);
    entities[entity_index.unwrap].jump();
}

void Game::reset_entities()
{
    static_assert(ROOM_ROW_COUNT > 0);
    entities[PLAYER_ENTITY_INDEX] = player_entity(vec2(200.0f, 200.0f));
}

void Game::entity_resolve_collision(Entity_Index entity_index)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);
    Entity *entity = &entities[entity_index.unwrap];

    if (entity->state == Entity_State::Alive) {
        const float step_x = entity->hitbox_local.w / (float) ENTITY_MESH_COLS;
        const float step_y = entity->hitbox_local.h / (float) ENTITY_MESH_ROWS;

        for (int rows = 0; rows <= ENTITY_MESH_ROWS; ++rows) {
            for (int cols = 0; cols <= ENTITY_MESH_COLS; ++cols) {
                Vec2f t0 = entity->pos +
                    vec2(entity->hitbox_local.x, entity->hitbox_local.y) +
                    vec2(cols * step_x, rows * step_y);
                Vec2f t1 = t0;

                grid.resolve_point_collision(&t1);

                Vec2f d = t1 - t0;

                const int IMPACT_THRESHOLD = 5;
                if (abs(d.y) >= IMPACT_THRESHOLD && !entity->has_jumped) entity->vel.y = 0;
                if (abs(d.x) >= IMPACT_THRESHOLD) entity->vel.x = 0;

                entity->pos += d;
            }
        }
    }
}

void Game::spawn_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter)
{
    for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
        if (projectiles[i].state == Projectile_State::Ded) {
            projectiles[i].state = Projectile_State::Active;
            projectiles[i].pos = pos;
            projectiles[i].vel = vel;
            projectiles[i].shooter = shooter;
            projectiles[i].lifetime = PROJECTILE_LIFETIME;
            projectiles[i].active_animat = projectile_active_animat;
            projectiles[i].poof_animat = projectile_poof_animat;
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
    // TODO(#150): the FPS is recalculated way too often which makes it pretty hard to read
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, PADDING),
             "FPS: %d", fps);
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, 50 + PADDING),
             "Mouse Position: (%.4f, %.4f)",
             mouse_position.x,
             mouse_position.y);
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, 2 * 50 + PADDING),
             "Collision Probe: (%.4f, %.4f)",
             collision_probe.x,
             collision_probe.y);
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, 3 * 50 + PADDING),
             "Projectiles: %d",
             count_alive_projectiles());
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, 4 * 50 + PADDING),
             "Player position: (%.4f, %.4f)",
             entities[PLAYER_ENTITY_INDEX].pos.x,
             entities[PLAYER_ENTITY_INDEX].pos.y);
    displayf(renderer, &debug_font,
             FONT_DEBUG_COLOR,
             FONT_SHADOW_COLOR,
             vec2(PADDING, 5 * 50 + PADDING),
             "Player velocity: (%.4f, %.4f)",
             entities[PLAYER_ENTITY_INDEX].vel.x,
             entities[PLAYER_ENTITY_INDEX].vel.y);

    if (tracking_projectile.has_value) {
        auto projectile = projectiles[tracking_projectile.unwrap.unwrap];
        const float SECOND_COLUMN_OFFSET = 700.0f;
        const SDL_Color TRACKING_DEBUG_COLOR = {255, 255, 150, 255};
        displayf(renderer, &debug_font,
                 TRACKING_DEBUG_COLOR,
                 FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, PADDING),
                 "State: %s", projectile_state_as_cstr(projectile.state));
        displayf(renderer, &debug_font,
                 TRACKING_DEBUG_COLOR,
                 FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, 50 + PADDING),
                 "Position: (%.4f, %.4f)",
                 projectile.pos.x, projectile.pos.y);
        displayf(renderer, &debug_font,
                 TRACKING_DEBUG_COLOR,
                 FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, 2 * 50 + PADDING),
                 "Velocity: (%.4f, %.4f)",
                 projectile.vel.x, projectile.vel.y);
        displayf(renderer, &debug_font,
                 TRACKING_DEBUG_COLOR,
                 FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, 3 * 50 + PADDING),
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

        entities[i].render_debug(renderer, camera);
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
        switch (projectiles[i].state) {
        case Projectile_State::Active: {
            projectiles[i].active_animat.render(
                renderer,
                camera.to_screen(projectiles[i].pos));
        } break;

        case Projectile_State::Poof: {
            projectiles[i].poof_animat.render(
                renderer,
                camera.to_screen(projectiles[i].pos));
        } break;

        case Projectile_State::Ded: {} break;
        }
    }
}

void Game::update_projectiles(float dt)
{
    for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
        switch (projectiles[i].state) {
        case Projectile_State::Active: {
            projectiles[i].active_animat.update(dt);
            projectiles[i].pos += projectiles[i].vel * dt;

            auto tile = grid.tile_at_abs(projectiles[i].pos);
            if (tile && tile_defs[*tile].is_collidable) {
                projectiles[i].kill();
                if (TILE_DESTROYABLE_0 <= *tile && *tile < TILE_DESTROYABLE_3) {
                    *tile += 1;
                } else if (*tile == TILE_DESTROYABLE_3) {
                    *tile = TILE_EMPTY;
                }
            }

            projectiles[i].lifetime -= dt;

            if (projectiles[i].lifetime <= 0.0f) {
                projectiles[i].kill();
            }
        } break;

        case Projectile_State::Poof: {
            projectiles[i].poof_animat.update(dt);
            if (projectiles[i].poof_animat.frame_current ==
                (projectiles[i].poof_animat.frame_count - 1)) {
                projectiles[i].state = Projectile_State::Ded;
            }
        } break;

        case Projectile_State::Ded: {} break;
        }
    }
}

const float PROJECTILE_TRACKING_PADDING = 50.0f;

Rectf Game::hitbox_of_projectile(Projectile_Index index)
{
    assert(index.unwrap < PROJECTILES_COUNT);
    return Rectf {
        projectiles[index.unwrap].pos.x - PROJECTILE_TRACKING_PADDING * 0.5f,
            projectiles[index.unwrap].pos.y - PROJECTILE_TRACKING_PADDING * 0.5f,
            PROJECTILE_TRACKING_PADDING,
            PROJECTILE_TRACKING_PADDING
            };
}

Maybe<Projectile_Index> Game::projectile_at_position(Vec2f position)
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

void Game::spawn_enemy_at(Vec2f pos)
{
    for (size_t i = PLAYER_ENTITY_INDEX + ENEMY_ENTITY_INDEX_OFFSET; i < ENTITIES_COUNT; ++i) {
        if (entities[i].state == Entity_State::Ded) {
            entities[i] = enemy_entity(pos);
            break;
        }
    }
}
