#include "something_game.hpp"

const SDL_Color DEBUG_FONT_COLOR = {220, 220, 220, 255};
const SDL_Color SUCCESS_FONT_COLOR = {150, 255, 150, 255};
const SDL_Color FAILURE_FONT_COLOR = {255, 150, 150, 255};
const SDL_Color DEBUG_FONT_SHADOW_COLOR = {0, 0, 0, 255};

const char *projectile_state_as_cstr(Projectile_State state)
{
    switch (state) {
    case Projectile_State::Ded: return "Ded";
    case Projectile_State::Active: return "Active";
    case Projectile_State::Poof: return "Poof";
    }

    assert(0 && "Incorrect Projectile_State");
}

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
void displayf(SDL_Renderer *renderer,
              TTF_Font *font,
              SDL_Color color,
              SDL_Color shadow_color,
              Vec2f p,
              const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char text[256];
    vsnprintf(text, sizeof(text), format, args);

    SDL_Texture *texture =
        render_text_as_texture(renderer, font, text, color);
    SDL_Texture *shadow_texture =
        render_text_as_texture(renderer, font, text, shadow_color);

    render_texture(renderer, shadow_texture, p - vec2(2.0f, 2.0f));
    render_texture(renderer, texture, p);
    SDL_DestroyTexture(texture);

    va_end(args);
}

void Projectile::kill()
{
    if (state == Projectile_State::Active) {
        state = Projectile_State::Poof;
        poof_animat.reset();
    }
}

void Game::update(float dt)
{
    // Update Player's gun direction //////////////////////////////
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    entities[PLAYER_ENTITY_INDEX].point_gun_at(
        camera.to_world(vec2((float) mouse_x, (float) mouse_y)));

    // Enemy AI //////////////////////////////
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

    // Update All Entities //////////////////////////////
    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        entities[i].update(dt, room_row, ROOM_ROW_COUNT);
    }

    // Update All Projectiles //////////////////////////////
    update_projectiles(dt);

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
                entity->lives -= CONFIG_INT(ENTITY_PROJECTILE_DAMAGE);
                entity->vel += normalize(projectile->vel) * CONFIG_FLOAT(ENTITY_PROJECTILE_KNOCKBACK);

                if (entity->lives <= 0) {
                    entity->kill();
                }
            }
        }
    }

    // Player Movement //////////////////////////////
    const float ENTITY_ACCEL = CONFIG_FLOAT(ENTITY_SPEED) * CONFIG_FLOAT(ENTITY_ACCEL_FACTOR);
    if (keyboard[SDL_SCANCODE_D]) {
        entities[PLAYER_ENTITY_INDEX].vel.x =
            fminf(
                entities[PLAYER_ENTITY_INDEX].vel.x + ENTITY_ACCEL * dt,
                CONFIG_FLOAT(ENTITY_SPEED));
        entities[PLAYER_ENTITY_INDEX].alive_state = Alive_State::Walking;
    } else if (keyboard[SDL_SCANCODE_A]) {
        entities[PLAYER_ENTITY_INDEX].vel.x =
            fmax(
                entities[PLAYER_ENTITY_INDEX].vel.x - ENTITY_ACCEL * dt,
                -CONFIG_FLOAT(ENTITY_SPEED));
        entities[PLAYER_ENTITY_INDEX].alive_state = Alive_State::Walking;
    } else {
        entities[PLAYER_ENTITY_INDEX].alive_state = Alive_State::Idle;
    }

    // Camera "Physics" //////////////////////////////
    const float PLAYER_CAMERA_FORCE = 2.0f;
    const float CENTER_CAMERA_FORCE = PLAYER_CAMERA_FORCE * 2.0f;

    const auto player_pos = entities[PLAYER_ENTITY_INDEX].pos;
    const auto room_center = room_row[room_index_at(player_pos).unwrap].center();

    camera.vel =
        (player_pos - camera.pos) * PLAYER_CAMERA_FORCE +
        (room_center - camera.pos) * CENTER_CAMERA_FORCE;
    camera.update(dt);

    // Popup //////////////////////////////
    popup.update(dt);
}

void Game::render(SDL_Renderer *renderer)
{
    auto index = room_index_at(entities[PLAYER_ENTITY_INDEX].pos);

    if (index.unwrap > 0) {
        room_row[index.unwrap - 1].render(
            renderer,
            camera,
            {0, 0, 0, (Uint8) CONFIG_INT(ROOM_NEIGHBOR_DIM_ALPHA)});
    }

    room_row[index.unwrap].render(renderer, camera);

    if (index.unwrap + 1 < (int) ROOM_ROW_COUNT) {
        room_row[index.unwrap + 1].render(
            renderer,
            camera,
            {0, 0, 0, (Uint8) CONFIG_INT(ROOM_NEIGHBOR_DIM_ALPHA)});
    }

    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        entities[i].render(renderer, camera);
    }

    render_projectiles(renderer, camera);

    popup.render(renderer, &camera);
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
    entity->cooldown_weapon = CONFIG_FLOAT(ENTITY_COOLDOWN_WEAPON);

    mixer.play_sample(entity->shoot_sample);
}

void Game::entity_jump(Entity_Index entity_index)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);
    entities[entity_index.unwrap].jump(&mixer);
}

const size_t ENTITY_FILE_CAPACITY = 1 * 1024 * 1024;
static char entity_file_buffer[ENTITY_FILE_CAPACITY];

const float POOF_DURATION = 0.2f;

void Game::inplace_spawn_entity_from_file(Entity_Index index, Vec2f pos,
                                          const char *file_path)
{
    entities[index.unwrap].lives = CONFIG_INT(ENTITY_INITIAL_LIVES);
    entities[index.unwrap].state = Entity_State::Alive;
    entities[index.unwrap].alive_state = Alive_State::Idle;
    entities[index.unwrap].pos = pos;
    entities[index.unwrap].gun_dir = vec2(1.0f, 0.0f);
    entities[index.unwrap].poof.duration = POOF_DURATION;

    entities[index.unwrap].prepare_for_jump_animat.begin = 0.0f;
    entities[index.unwrap].prepare_for_jump_animat.end = 0.2f;
    entities[index.unwrap].prepare_for_jump_animat.duration = 0.2f;

    entities[index.unwrap].jump_animat.rubber_animats[0].begin = 0.2f;
    entities[index.unwrap].jump_animat.rubber_animats[0].end = -0.2f;
    entities[index.unwrap].jump_animat.rubber_animats[0].duration = 0.1f;

    entities[index.unwrap].jump_animat.rubber_animats[1].begin = -0.2f;
    entities[index.unwrap].jump_animat.rubber_animats[1].end = 0.0f;
    entities[index.unwrap].jump_animat.rubber_animats[1].duration = 0.2f;

    auto input = file_as_string_view(file_path,
                                     entity_file_buffer,
                                     ENTITY_FILE_CAPACITY);

    for (size_t line_number = 0; input.count > 0; ++line_number) {
        auto fail_parsing =
            [&file_path, &line_number](auto ...args) {
                println(stderr, file_path, ":", line_number, ": ", args...);
                abort();
            };

        auto line = input.chop_by_delim('\n').trim();
        if (line.count == 0) continue; // Empty line
        if (*line.data == '#') continue; // Comment
        auto key = line.chop_by_delim('=').trim();
        auto value = line.chop_by_delim('#').trim(); // Potential comment on the same line

        auto subkey = key.chop_by_delim('.');
        if (subkey == "texbox"_sv) {
            subkey = key.chop_by_delim('.');
            auto x = value.as_float();
            if (!x.has_value) fail_parsing(value, " is not a number");

            if (subkey == "w"_sv) {
                entities[index.unwrap].texbox_local.w = x.unwrap;
            } else if (subkey == "h"_sv) {
                entities[index.unwrap].texbox_local.h = x.unwrap;
            } else {
                fail_parsing("`texbox` does not have `", subkey, "`subkey");
            }
        } else if (subkey == "hitbox"_sv) {
            subkey = key.chop_by_delim('.');
            auto x = value.as_float();
            if (!x.has_value) fail_parsing(value, " is not a number");

            if (subkey == "w"_sv) {
                entities[index.unwrap].hitbox_local.w = x.unwrap;
            } else if (subkey == "h"_sv) {
                entities[index.unwrap].hitbox_local.h = x.unwrap;
            } else {
                fail_parsing("`hitbox` does not have `", subkey, "`subkey");
            }
        } else if (subkey == "idle"_sv) {
            entities[index.unwrap].idle = frame_animat_by_name(value);
        } else if (subkey == "walking"_sv) {
            entities[index.unwrap].walking = frame_animat_by_name(value);
        } else if (subkey == "jump_sample"_sv) {
            subkey = key.chop_by_delim('.');
            auto jump_index = subkey.as_integer<int>();
            if (!jump_index.has_value) {
                fail_parsing(subkey, " is not a correct number");
            }
            if (jump_index.unwrap < 0 || (size_t) jump_index.unwrap >= JUMP_SAMPLES_CAPACITY) {
                fail_parsing(subkey, " is out-of-bound with jump_sample");
            }
            entities[index.unwrap].jump_samples[jump_index.unwrap] =
                sample_s16_by_name(value);
        } else if (subkey == "shoot_sample"_sv) {
            entities[index.unwrap].shoot_sample = sample_s16_by_name(value);
        } else {
            fail_parsing("Unknown entity description subkey `", subkey, "`");
        }
    }

    entities[index.unwrap].texbox_local.x = entities[index.unwrap].texbox_local.w * -0.5f;
    entities[index.unwrap].texbox_local.y = entities[index.unwrap].texbox_local.h * -0.5f;
    entities[index.unwrap].hitbox_local.x = entities[index.unwrap].hitbox_local.w * -0.5f;
    entities[index.unwrap].hitbox_local.y = entities[index.unwrap].hitbox_local.h * -0.5f;
}

void Game::inplace_spawn_entity(Entity_Index index,
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

    memset(entities + index.unwrap, 0, sizeof(Entity));
    entities[index.unwrap].lives = CONFIG_INT(ENTITY_INITIAL_LIVES);
    entities[index.unwrap].state = Entity_State::Alive;
    entities[index.unwrap].alive_state = Alive_State::Idle;
    entities[index.unwrap].texbox_local = texbox_local;
    entities[index.unwrap].hitbox_local = hitbox_local;
    entities[index.unwrap].pos = pos;
    entities[index.unwrap].gun_dir = vec2(1.0f, 0.0f);
    entities[index.unwrap].poof.duration = POOF_DURATION;

    entities[index.unwrap].walking = entity_walking_animat;
    entities[index.unwrap].idle = entity_idle_animat;

    entities[index.unwrap].prepare_for_jump_animat.begin = 0.0f;
    entities[index.unwrap].prepare_for_jump_animat.end = 0.2f;
    entities[index.unwrap].prepare_for_jump_animat.duration = 0.2f;

    entities[index.unwrap].jump_animat.rubber_animats[0].begin = 0.2f;
    entities[index.unwrap].jump_animat.rubber_animats[0].end = -0.2f;
    entities[index.unwrap].jump_animat.rubber_animats[0].duration = 0.1f;

    entities[index.unwrap].jump_animat.rubber_animats[1].begin = -0.2f;
    entities[index.unwrap].jump_animat.rubber_animats[1].end = 0.0f;
    entities[index.unwrap].jump_animat.rubber_animats[1].duration = 0.2f;

    entities[index.unwrap].jump_samples[0] = entity_jump_sample1;
    entities[index.unwrap].jump_samples[1] = entity_jump_sample2;
}

void Game::reset_entities()
{
    static_assert(ROOM_ROW_COUNT > 0);
    inplace_spawn_entity_from_file({PLAYER_ENTITY_INDEX},
                                   room_row[0].center(),
                                   "./assets/entities/player.txt");
    entities[PLAYER_ENTITY_INDEX].shoot_sample = player_shoot_sample;

    // TODO(#84): load enemies from description files
    for (size_t i = 0; i < ROOM_ROW_COUNT - 1; ++i) {
        inplace_spawn_entity({ENEMY_ENTITY_INDEX_OFFSET + i},
                             room_row[i + 1].center());
    }
}

void Game::spawn_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter)
{
    const float PROJECTILE_LIFETIME = 5.0f;
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

void Game::render_debug_overlay(SDL_Renderer *renderer)
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
    displayf(renderer, debug_font,
             DEBUG_FONT_COLOR,
             DEBUG_FONT_SHADOW_COLOR,
             vec2(PADDING, PADDING),
             "FPS: %d", 60);
    displayf(renderer, debug_font,
             DEBUG_FONT_COLOR,
             DEBUG_FONT_SHADOW_COLOR,
             vec2(PADDING, 50 + PADDING),
             "Mouse Position: (%.4f, %.4f)",
             debug_mouse_position.x,
             debug_mouse_position.y);
    displayf(renderer, debug_font,
             DEBUG_FONT_COLOR,
             DEBUG_FONT_SHADOW_COLOR,
             vec2(PADDING, 2 * 50 + PADDING),
             "Collision Probe: (%.4f, %.4f)",
             collision_probe.x,
             collision_probe.y);
    displayf(renderer, debug_font,
             DEBUG_FONT_COLOR,
             DEBUG_FONT_SHADOW_COLOR,
             vec2(PADDING, 3 * 50 + PADDING),
             "Projectiles: %d",
             count_alive_projectiles());
    displayf(renderer, debug_font,
             DEBUG_FONT_COLOR,
             DEBUG_FONT_SHADOW_COLOR,
             vec2(PADDING, 4 * 50 + PADDING),
             "Player position: (%.4f, %.4f)",
             entities[PLAYER_ENTITY_INDEX].pos.x,
             entities[PLAYER_ENTITY_INDEX].pos.y);
    displayf(renderer, debug_font,
             DEBUG_FONT_COLOR,
             DEBUG_FONT_SHADOW_COLOR,
             vec2(PADDING, 5 * 50 + PADDING),
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
        const SDL_Color TRACKING_DEBUG_COLOR = {255, 255, 150, 255};
        displayf(renderer, debug_font,
                 TRACKING_DEBUG_COLOR,
                 DEBUG_FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, PADDING),
                 "State: %s", projectile_state_as_cstr(projectile.state));
        displayf(renderer, debug_font,
                 TRACKING_DEBUG_COLOR,
                 DEBUG_FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, 50 + PADDING),
                 "Position: (%.4f, %.4f)",
                 projectile.pos.x, projectile.pos.y);
        displayf(renderer, debug_font,
                 TRACKING_DEBUG_COLOR,
                 DEBUG_FONT_SHADOW_COLOR,
                 vec2(PADDING + SECOND_COLUMN_OFFSET, 2 * 50 + PADDING),
                 "Velocity: (%.4f, %.4f)",
                 projectile.vel.x, projectile.vel.y);
        displayf(renderer, debug_font,
                 TRACKING_DEBUG_COLOR,
                 DEBUG_FONT_SHADOW_COLOR,
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

void Game::update_projectiles(float dt)
{
    for (size_t i = 0; i < PROJECTILES_COUNT; ++i) {
        switch (projectiles[i].state) {
        case Projectile_State::Active: {
            update_animat(&projectiles[i].active_animat, dt);
            projectiles[i].pos += projectiles[i].vel * dt;

            int room_current = (int) floorf(projectiles[i].pos.x / ROOM_BOUNDARY.w);
            if (0 <= room_current && room_current < (int) ROOM_ROW_COUNT) {
                auto tile = room_row[room_current].tile_at(projectiles[i].pos);
                if (tile && tile_defs[*tile].is_collidable) {
                    projectiles[i].kill();
                    if (TILE_DESTROYABLE_0 <= *tile && *tile < TILE_DESTROYABLE_3) {
                        *tile += 1;
                    } else if (*tile == TILE_DESTROYABLE_3) {
                        *tile = TILE_EMPTY;
                    }
                }
            }

            projectiles[i].lifetime -= dt;

            if (projectiles[i].lifetime <= 0.0f) {
                projectiles[i].kill();
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

Room_Index Game::room_index_at(Vec2f p)
{
    int index = (int) floor(p.x / ROOM_BOUNDARY.w);

    if (index < 0) return {0};
    if (index >= (int) ROOM_ROW_COUNT) return {ROOM_ROW_COUNT - 1};
    return {(size_t) index};
}

void Game::render_room_minimap(SDL_Renderer *renderer,
                               Room_Index index,
                               Vec2f position)
{
    assert(index.unwrap < ROOM_ROW_COUNT);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int row = 0; row < ROOM_HEIGHT; ++row) {
        for (int col = 0; col < ROOM_WIDTH; ++col) {
            if (room_row[index.unwrap].tiles[row][col] == TILE_WALL) {
                SDL_Rect rect = {
                    (int) (position.x + (float) col * MINIMAP_TILE_SIZE),
                    (int) (position.y + (float) row * MINIMAP_TILE_SIZE),
                    (int) MINIMAP_TILE_SIZE,
                    (int) MINIMAP_TILE_SIZE
                };
                sec(SDL_RenderFillRect(renderer, &rect));
            }
        }
    }
}

void Game::render_room_row_minimap(SDL_Renderer *renderer,
                                   Vec2f position)
{
    for (size_t i = 0; i < ROOM_ROW_COUNT; ++i) {
        render_room_minimap(
            renderer,
            {i},
            position + vec2(MINIMAP_ROOM_BOUNDARY.w * (float) i, 0.0f));
    }
}

void Game::render_entity_on_minimap(SDL_Renderer *renderer,
                                    Vec2f position,
                                    Vec2f entity_position)
{
    const Vec2f minimap_position =
        entity_position /
        vec2(ROOM_BOUNDARY.w, ROOM_BOUNDARY.h) *
        vec2((float) MINIMAP_ROOM_BOUNDARY.w, (float) MINIMAP_ROOM_BOUNDARY.h);

    const Vec2f screen_position =
        minimap_position + position;

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

    SDL_Rect rect = {
        (int) (screen_position.x - MINIMAP_ENTITY_SIZE * 0.5f),
        (int) (screen_position.y - MINIMAP_ENTITY_SIZE * 0.5f),
        (int) MINIMAP_ENTITY_SIZE,
        (int) MINIMAP_ENTITY_SIZE
    };
    sec(SDL_RenderFillRect(renderer, &rect));
}

void Popup::notify(SDL_Color color, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    buffer_size = vsnprintf(buffer, POPUP_BUFFER_CAPACITY, format, args);
    if (buffer_size < 0) {
        println(stderr, "[WARN] Popup::notify encountered an error");
    }

    a = 1.0f;
    this->color = color;
    va_end(args);
}

Vec2f shadow_offset_dir(TTF_Font *font, float ratio)
{
    int h = TTF_FontHeight(font);
    return vec2((float) h * ratio, (float) h * ratio);
}

void Popup::render(SDL_Renderer *renderer, const Camera *camera)
{
    if (buffer_size > 0 && a > 1e-6) {
        const Uint8 alpha      = (Uint8) floorf(255.0f * fminf(a, 1.0f));
        int w, h;
        stec(TTF_SizeText(font, buffer, &w, &h));
        SDL_Rect srcrect = {0, 0, w, h};
        SDL_Rect dstrect = {
            (int) floorf(camera->width  * 0.5f - (float) w * 0.5f),
            (int) floorf(camera->height * 0.5f - (float) h * 0.5f),
            w, h
        };

        // SHADOW //////////////////////////////
        SDL_Color shadow_color = DEBUG_FONT_SHADOW_COLOR;
        shadow_color.a         = alpha;

        SDL_Texture *shadow_texture =
            render_text_as_texture(renderer, font, buffer, shadow_color);

        SDL_Rect shadow_dstrect = dstrect;
        const Vec2f offset = shadow_offset_dir(font, 0.05f);
        shadow_dstrect.x -= (int) offset.x;
        shadow_dstrect.y -= (int) offset.y;

        sec(SDL_RenderCopy(renderer, shadow_texture, &srcrect, &shadow_dstrect));
        SDL_DestroyTexture(shadow_texture);

        // TEXT   //////////////////////////////
        SDL_Color front_color = color;
        color.a               = alpha;

        SDL_Texture *texture =
            render_text_as_texture(renderer, font, buffer, front_color);

        sec(SDL_RenderCopy(renderer, texture, &srcrect, &dstrect));
        SDL_DestroyTexture(texture);
    }
}

const float POPUP_FADEOUT_RATE = 0.5f;

void Popup::update(float delta_time)
{
    if (a > 0.0f) {
        a -= POPUP_FADEOUT_RATE * delta_time;
    }
}
