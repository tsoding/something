#include "./something_print.hpp"
#include "./something_entity.hpp"

void Entity::kill()
{
    if (state == Entity_State::Alive) {
        state = Entity_State::Poof;
    }
}

SDL_Color mix_colors(SDL_Color b32, SDL_Color a32)
{
    const float a32_alpha = a32.a / 255.0;
    const float b32_alpha = b32.a / 255.0;
    const float r_alpha = a32_alpha + b32_alpha * (1.0f - a32_alpha);

    SDL_Color r = {};

    r.r = (Uint8) ((a32.r * a32_alpha + b32.r * b32_alpha * (1.0f - a32_alpha)) / r_alpha);
    r.g = (Uint8) ((a32.g * a32_alpha + b32.g * b32_alpha * (1.0f - a32_alpha)) / r_alpha);
    r.b = (Uint8) ((a32.b * a32_alpha + b32.b * b32_alpha * (1.0f - a32_alpha)) / r_alpha);
    r.a = (Uint8) (r_alpha * 255.0f);

    return r;
}

void Entity::render(SDL_Renderer *renderer, Camera camera, SDL_Color shade) const
{
    const SDL_RendererFlip flip =
        gun_dir.x > 0.0f ?
        SDL_FLIP_NONE :
        SDL_FLIP_HORIZONTAL;

    // TODO(#185): should we use shade for the particles of an entity?
    particles.render(renderer, camera);

    switch (state) {
    case Entity_State::Alive: {
        // Figuring out texbox
        Rectf texbox = {};

        switch (jump_state) {
        case Jump_State::No_Jump:
            texbox = texbox_world();
            break;

        case Jump_State::Prepare:
            texbox = prepare_for_jump_animat.transform_rect(texbox_local, pos);
            break;

        case Jump_State::Jump:
            texbox = jump_animat.transform_rect(texbox_local, pos);
            break;
        }

        // Rendering Live Bar
        {
            const Rectf livebar_border = {
                texbox.x + texbox.w * 0.5f - ENTITY_LIVEBAR_WIDTH * 0.5f,
                texbox.y - ENTITY_LIVEBAR_HEIGHT - ENTITY_LIVEBAR_PADDING_BOTTOM,
                ENTITY_LIVEBAR_WIDTH,
                ENTITY_LIVEBAR_HEIGHT
            };
            const float percent = (float) lives / (float) ENTITY_MAX_LIVES;
            const Rectf livebar_remain = {
                livebar_border.x, livebar_border.y,
                ENTITY_LIVEBAR_WIDTH * percent,
                ENTITY_LIVEBAR_HEIGHT
            };

            if (percent > 0.75f) {
                sec(SDL_SetRenderDrawColor(
                        renderer,
                        ENTITY_LIVEBAR_FULL_COLOR.r,
                        ENTITY_LIVEBAR_FULL_COLOR.g,
                        ENTITY_LIVEBAR_FULL_COLOR.b,
                        ENTITY_LIVEBAR_FULL_COLOR.a));
            } else if (0.25f < percent && percent < 0.75f) {
                sec(SDL_SetRenderDrawColor(
                        renderer,
                        ENTITY_LIVEBAR_HALF_COLOR.r,
                        ENTITY_LIVEBAR_HALF_COLOR.g,
                        ENTITY_LIVEBAR_HALF_COLOR.b,
                        ENTITY_LIVEBAR_HALF_COLOR.a));
            } else {
                sec(SDL_SetRenderDrawColor(
                        renderer,
                        ENTITY_LIVEBAR_LOW_COLOR.r,
                        ENTITY_LIVEBAR_LOW_COLOR.g,
                        ENTITY_LIVEBAR_LOW_COLOR.b,
                        ENTITY_LIVEBAR_LOW_COLOR.a));
            }
            const auto rect_border = rectf_for_sdl(camera.to_screen(livebar_border));
            sec(SDL_RenderDrawRect(renderer, &rect_border));
            const auto rect_remain = rectf_for_sdl(camera.to_screen(livebar_remain));
            sec(SDL_RenderFillRect(renderer, &rect_remain));
        }

        SDL_Color effective_flash_color = flash_color;
        effective_flash_color.a = flash_alpha * 255.0f;

        // Render the character
        switch (alive_state) {
        case Alive_State::Idle: {
            idle.render(renderer, camera.to_screen(texbox), flip,
                        mix_colors(shade, effective_flash_color));
        } break;

        case Alive_State::Walking: {
            walking.render(renderer, camera.to_screen(texbox), flip,
                           mix_colors(shade, effective_flash_color));
        } break;
        }

        // Render the gun
        // TODO(#59): Proper gun rendering
        Vec2f gun_begin = pos;
        render_line(
            renderer,
            camera.to_screen(gun_begin),
            camera.to_screen(gun_begin + normalize(gun_dir) * ENTITY_GUN_LENGTH),
            {255, 0, 0, 255});
    } break;

    case Entity_State::Poof: {
        Rectf texbox = poof_animat.transform_rect(texbox_local, pos);
        // TODO(#151): Poof state loses last alive frame
        //   Previous animation implementation was capturing texture of last alive state.
        //   So if entity was shot in running pose it was squashing in this position.
        //   So there's no sudden graphical switch to idle texture.
        idle.render(renderer, camera.to_screen(texbox), flip, shade);
    } break;

    case Entity_State::Ded: {} break;
    }
}

void Entity::render_debug(SDL_Renderer *renderer, Camera camera) const
{
    if (state == Entity_State::Alive) {
        const float step_x = hitbox_local.w / (float) ENTITY_MESH_COLS;
        const float step_y = hitbox_local.h / (float) ENTITY_MESH_ROWS;

        for (int rows = 0; rows <= ENTITY_MESH_ROWS; ++rows) {
            for (int cols = 0; cols <= ENTITY_MESH_COLS; ++cols) {
                Vec2f t = camera.to_screen(
                    pos +
                    vec2(hitbox_local.x, hitbox_local.y) +
                    vec2(cols * step_x, rows * step_y));
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                const int PROBE_SIZE = 10;
                SDL_Rect rect = {
                    (int)t.x - PROBE_SIZE / 2,
                    (int)t.y - PROBE_SIZE / 2,
                    PROBE_SIZE,
                    PROBE_SIZE
                };
                sec(SDL_RenderFillRect(renderer, &rect));
            }
        }
    }
}

void Entity::update(float dt, Sample_Mixer *mixer, Tile_Grid *grid)
{
    // TODO(#197): introduce some particle "puffs" when jumping or landing
    if (state == Entity_State::Alive && alive_state == Alive_State::Walking && ground(grid)) {
        particles.state = Particles::EMITTING;

        const auto tile_sprite = tile_defs[*grid->tile_at_abs(feet() + vec2(0.0f, TILE_SIZE * 0.5f))].top_texture;
        const auto surface = surfaces[tile_sprite.texture_index.unwrap];
        const auto x = rand() % tile_sprite.srcrect.w;
        sec(SDL_LockSurface(surface));
        {
            assert(surface->format->format == SDL_PIXELFORMAT_RGBA32);
            const auto pixel = *(Uint32*) ((uint8_t *) surface->pixels + tile_sprite.srcrect.y * surface->pitch + (tile_sprite.srcrect.x + x) * sizeof(Uint32));
            SDL_Color color = {};
            SDL_GetRGBA(
                pixel,
                surface->format,
                &color.r,
                &color.g,
                &color.b,
                &color.a);
            particles.current_color = sdl_to_rgba(color).to_hsla();
        }
        SDL_UnlockSurface(surface);
    } else {
        particles.state = Particles::DISABLED;
    }

    particles.update(dt, feet(), grid);

    switch (state) {
    case Entity_State::Alive: {
        flash_alpha = fmax(0.0f, flash_alpha - ENTITY_FLASH_ALPHA_DECAY * dt);

        vel.y += ENTITY_GRAVITY * dt;

        const float ENTITY_DECEL = ENTITY_SPEED * ENTITY_DECEL_FACTOR;
        const float ENTITY_STOP_THRESHOLD = 100.0f;
        if (fabs(vel.x) > ENTITY_STOP_THRESHOLD) {
            vel.x -= sgn(vel.x) * ENTITY_DECEL * dt;
        } else {
            vel.x = 0.0f;
        }

        pos += vel * dt;
        cooldown_weapon -= dt;

        switch (jump_state) {
        case Jump_State::No_Jump:
            break;

        case Jump_State::Prepare:
            prepare_for_jump_animat.update(dt);
            if (prepare_for_jump_animat.finished()) {
                jump_animat.reset();
                jump_state = Jump_State::Jump;
                has_jumped = true;
                vel.y = ENTITY_GRAVITY * -0.6f;
                mixer->play_sample(jump_samples[rand() % 2]);
            }
            break;

        case Jump_State::Jump:
            jump_animat.update(dt);
            if (jump_animat.finished()) {
                jump_state = Jump_State::No_Jump;
            }
            break;
        }

        switch (alive_state) {
        case Alive_State::Idle:
            idle.update(dt);
            break;

        case Alive_State::Walking:
            const float ENTITY_ACCEL = ENTITY_SPEED * ENTITY_ACCEL_FACTOR;
            switch (walking_direction) {
            case Left: {
                vel.x = fmax(vel.x - ENTITY_ACCEL * dt,
                             -ENTITY_SPEED);
            } break;

            case Right: {
                vel.x = fminf(vel.x + ENTITY_ACCEL * dt,
                              ENTITY_SPEED);
            } break;
            }

            walking.update(dt);
            break;
        }
    } break;

    case Entity_State::Poof: {
        poof_animat.update(dt);
        if (poof_animat.finished()) {
            state = Entity_State::Ded;
        }
    } break;

    case Entity_State::Ded: {} break;
    }
}

void Entity::point_gun_at(Vec2f target)
{
    gun_dir = normalize(target - pos);
}

void Entity::jump()
{
    if (state == Entity_State::Alive) {
        if (jump_state == Jump_State::No_Jump) {
            prepare_for_jump_animat.reset();
            jump_state = Jump_State::Prepare;
        }
    }
}

Entity player_entity(Vec2f pos)
{
    Entity entity = {};

    entity.texbox_local.w = PLAYER_TEXBOX_W;
    entity.texbox_local.h = PLAYER_TEXBOX_H;
    entity.hitbox_local.w = PLAYER_HITBOX_W;
    entity.hitbox_local.h = PLAYER_HITBOX_H;
    entity.texbox_local.x = entity.texbox_local.w * -0.5f;
    entity.texbox_local.y = entity.texbox_local.h * -0.5f;
    entity.hitbox_local.x = entity.hitbox_local.w * -0.5f;
    entity.hitbox_local.y = entity.hitbox_local.h * -0.5f;


    entity.idle = frame_animat_by_name(PLAYER_IDLE);
    entity.walking = frame_animat_by_name(PLAYER_WALKING);
    entity.jump_samples[0] = sample_s16_by_name(PLAYER_JUMP_SAMPLE_0);
    entity.jump_samples[1] = sample_s16_by_name(PLAYER_JUMP_SAMPLE_1);
    entity.shoot_sample = sample_s16_by_name(PLAYER_SHOOT_SAMPLE);

    entity.lives = ENTITY_INITIAL_LIVES;
    entity.state = Entity_State::Alive;
    entity.alive_state = Alive_State::Idle;
    entity.pos = pos;
    entity.gun_dir = vec2(1.0f, 0.0f);

    entity.prepare_for_jump_animat.begin = 0.0f;
    entity.prepare_for_jump_animat.end = 0.2f;
    entity.prepare_for_jump_animat.duration = 0.05f;

    entity.jump_animat.rubber_animats[0].begin = 0.2f;
    entity.jump_animat.rubber_animats[0].end = -0.2f;
    entity.jump_animat.rubber_animats[0].duration = 0.1f;

    entity.jump_animat.rubber_animats[1].begin = -0.2f;
    entity.jump_animat.rubber_animats[1].end = 0.0f;
    entity.jump_animat.rubber_animats[1].duration = 0.2f;

    entity.poof_animat.begin = 0.0f;
    entity.poof_animat.end = 1.0f;
    entity.poof_animat.duration = 0.1f;

    return entity;
}

Entity enemy_entity(Vec2f pos)
{
    Entity entity = {};

    entity.texbox_local.w = ENEMY_TEXBOX_W;
    entity.texbox_local.h = ENEMY_TEXBOX_H;
    entity.hitbox_local.w = ENEMY_HITBOX_W;
    entity.hitbox_local.h = ENEMY_HITBOX_H;
    entity.texbox_local.x = entity.texbox_local.w * -0.5f;
    entity.texbox_local.y = entity.texbox_local.h * -0.5f;
    entity.hitbox_local.x = entity.hitbox_local.w * -0.5f;
    entity.hitbox_local.y = entity.hitbox_local.h * -0.5f;


    entity.idle = frame_animat_by_name(ENEMY_IDLE);
    entity.walking = frame_animat_by_name(ENEMY_WALKING);
    entity.jump_samples[0] = sample_s16_by_name(ENEMY_JUMP_SAMPLE_0);
    entity.jump_samples[1] = sample_s16_by_name(ENEMY_JUMP_SAMPLE_1);

    entity.lives = ENTITY_INITIAL_LIVES;
    entity.state = Entity_State::Alive;
    entity.alive_state = Alive_State::Idle;
    entity.pos = pos;
    entity.gun_dir = vec2(1.0f, 0.0f);

    entity.prepare_for_jump_animat.begin = 0.0f;
    entity.prepare_for_jump_animat.end = 0.2f;
    entity.prepare_for_jump_animat.duration = 0.2f;

    entity.jump_animat.rubber_animats[0].begin = 0.2f;
    entity.jump_animat.rubber_animats[0].end = -0.2f;
    entity.jump_animat.rubber_animats[0].duration = 0.1f;

    entity.jump_animat.rubber_animats[1].begin = -0.2f;
    entity.jump_animat.rubber_animats[1].end = 0.0f;
    entity.jump_animat.rubber_animats[1].duration = 0.2f;

    entity.poof_animat.begin = 0.0f;
    entity.poof_animat.end = 1.0f;
    entity.poof_animat.duration = 0.1f;

    return entity;
}

void Entity::flash(SDL_Color color)
{
    flash_alpha = 1.0f;
    flash_color = color;
}

void Entity::move(Direction direction)
{
    alive_state = Alive_State::Walking;
    walking_direction = direction;
}

void Entity::stop()
{
    alive_state = Alive_State::Idle;
}

Vec2f Entity::feet()
{
    auto hitbox = hitbox_local;
    hitbox.x += pos.x;
    hitbox.y += pos.y;
    return vec2(hitbox.x, hitbox.y) + vec2(0.5f, 1.0f) * vec2(hitbox.w, hitbox.h);
}

bool Entity::ground(Tile_Grid *grid)
{
    return !grid->is_tile_empty_abs(feet() + vec2(0.0f, TILE_SIZE * 0.5f));
}
