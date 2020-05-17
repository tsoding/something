#include "./something_entity.hpp"

void render_line(SDL_Renderer *renderer, Vec2f begin, Vec2f end)
{
    sec(SDL_RenderDrawLine(
            renderer,
            (int) floorf(begin.x), (int) floorf(begin.y),
            (int) floorf(end.x),   (int) floorf(end.y)));
}

void Entity::resolve_entity_collision(Room *room_row, size_t room_row_count)
{
    Vec2f p0 = vec2(hitbox_local.x, hitbox_local.y) + pos;
    Vec2f p1 = p0 + vec2(hitbox_local.w, hitbox_local.h);

    Vec2f mesh[] = {
        p0,
        {p1.x, p0.y},
        {p0.x, p1.y},
        p1,
    };
    const int MESH_COUNT = sizeof(mesh) / sizeof(mesh[0]);

    for (int i = 0; i < MESH_COUNT; ++i) {
        Vec2f t = mesh[i];
        int room_index = (int) floorf(t.x / ROOM_BOUNDARY.w);

        if (0 <= room_index && room_index < (int) room_row_count) {
            room_row[room_index].resolve_point_collision(&t);
        }

        Vec2f d = t - mesh[i];

        const int IMPACT_THRESHOLD = 5;
        if (abs(d.y) >= IMPACT_THRESHOLD) vel.y = 0;
        if (abs(d.x) >= IMPACT_THRESHOLD) vel.x = 0;

        for (int j = 0; j < MESH_COUNT; ++j) {
            mesh[j] += d;
        }

        pos += d;
    }
}

void Entity::kill()
{
    if (state == Entity_State::Alive) {
        poof.a = 0.0f;

        switch (alive_state) {
        case Alive_State::Idle:
            if (idle.frame_current < idle.frame_count) {
                poof.sprite = idle.frames[idle.frame_current];
            }
            break;
        case Alive_State::Walking:
            if (walking.frame_current < walking.frame_count) {
                poof.sprite = walking.frames[walking.frame_current];
            }
            break;
        }

        state = Entity_State::Poof;
    }
}

void Entity::render(SDL_Renderer *renderer, Camera camera) const
{
    const SDL_RendererFlip flip =
        gun_dir.x > 0.0f ?
        SDL_FLIP_NONE :
        SDL_FLIP_HORIZONTAL;

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

        // Render the character
        switch (alive_state) {
        case Alive_State::Idle: {
            render_animat(renderer, idle, camera.to_screen(texbox), flip);
        } break;

        case Alive_State::Walking: {
            render_animat(renderer, walking, camera.to_screen(texbox), flip);
        } break;
        }

        // Render the gun
        // TODO(#59): Proper gun rendering
        Vec2f gun_begin = pos;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        render_line(
            renderer,
            camera.to_screen(gun_begin),
            camera.to_screen(gun_begin + normalize(gun_dir) * ENTITY_GUN_LENGTH));
    } break;

    case Entity_State::Poof: {
        poof.render(renderer, camera.to_screen(pos), camera.to_screen(texbox_world()), flip);
    } break;

    case Entity_State::Ded: {} break;
    }
}

void Entity::update(float dt, Room *room_row, size_t room_row_count)
{
    switch (state) {
    case Entity_State::Alive: {
        vel.y += ENTITY_GRAVITY * dt;

        const float ENTITY_DECEL = ENTITY_SPEED * ENTITY_DECEL_FACTOR;
        const float ENTITY_STOP_THRESHOLD = 100.0f;
        if (fabs(vel.x) > ENTITY_STOP_THRESHOLD) {
            vel.x -= sgn(vel.x) * ENTITY_DECEL * dt;
        } else {
            vel.x = 0.0f;
        }

        pos += vel * dt;
        resolve_entity_collision(room_row, room_row_count);
        cooldown_weapon -= dt;

        switch (jump_state) {
        case Jump_State::No_Jump:
            break;

        case Jump_State::Prepare:
            prepare_for_jump_animat.update(dt);
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
            update_animat(&idle, dt);
            break;

        case Alive_State::Walking:
            update_animat(&walking, dt);
            break;
        }
    } break;

    case Entity_State::Poof: {
        poof.update(dt);
        if (poof.a >= 1) {
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

void Entity::jump(Sample_Mixer *mixer)
{
    if (state == Entity_State::Alive) {
        switch (jump_state) {
        case Jump_State::No_Jump: {
            prepare_for_jump_animat.reset();
            jump_state = Jump_State::Prepare;
        } break;

        case Jump_State::Prepare: {
            float a = prepare_for_jump_animat.t / prepare_for_jump_animat.duration;
            jump_animat.reset();
            jump_state = Jump_State::Jump;
            vel.y = ENTITY_GRAVITY * -min(a, 0.6f);
            mixer->play_sample(jump_samples[rand() % 2]);
        } break;

        case Jump_State::Jump:
            break;
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
    entity.poof.duration = POOF_DURATION;

    entity.prepare_for_jump_animat.begin = 0.0f;
    entity.prepare_for_jump_animat.end = 0.2f;
    entity.prepare_for_jump_animat.duration = 0.2f;

    entity.jump_animat.rubber_animats[0].begin = 0.2f;
    entity.jump_animat.rubber_animats[0].end = -0.2f;
    entity.jump_animat.rubber_animats[0].duration = 0.1f;

    entity.jump_animat.rubber_animats[1].begin = -0.2f;
    entity.jump_animat.rubber_animats[1].end = 0.0f;
    entity.jump_animat.rubber_animats[1].duration = 0.2f;

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
    entity.poof.duration = POOF_DURATION;

    entity.prepare_for_jump_animat.begin = 0.0f;
    entity.prepare_for_jump_animat.end = 0.2f;
    entity.prepare_for_jump_animat.duration = 0.2f;

    entity.jump_animat.rubber_animats[0].begin = 0.2f;
    entity.jump_animat.rubber_animats[0].end = -0.2f;
    entity.jump_animat.rubber_animats[0].duration = 0.1f;

    entity.jump_animat.rubber_animats[1].begin = -0.2f;
    entity.jump_animat.rubber_animats[1].end = 0.0f;
    entity.jump_animat.rubber_animats[1].duration = 0.2f;

    return entity;
}
