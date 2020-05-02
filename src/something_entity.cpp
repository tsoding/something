#include "./something_entity.hpp"

void render_line(SDL_Renderer *renderer, Vec2f begin, Vec2f end)
{
    sec(SDL_RenderDrawLine(
            renderer,
            (int) floorf(begin.x), (int) floorf(begin.y),
            (int) floorf(end.x),   (int) floorf(end.y)));
}

void Entity::resolve_entity_collision()
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

        if (0 <= room_index && room_index < (int) ROOM_ROW_COUNT) {
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

        switch (alive_state) {
        case Alive_State::Idle: {
            render_animat(renderer, idle, camera.to_screen(texbox), flip);
        } break;

        case Alive_State::Walking: {
            render_animat(renderer, walking, camera.to_screen(texbox), flip);
        } break;
        }

        // TODO(#61): file with variables
        const float GUN_LENGTH = 50.0f;

        // TODO(#59): Proper gun rendering

        Vec2f gun_begin = pos;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        render_line(
            renderer,
            camera.to_screen(gun_begin),
            camera.to_screen(gun_begin + normalize(gun_dir) * GUN_LENGTH));
    } break;

    case Entity_State::Poof: {
        poof.render(renderer, camera.to_screen(pos), camera.to_screen(texbox_world()), flip);
    } break;

    case Entity_State::Ded: {} break;
    }
}

void Entity::update(Vec2f gravity, float dt)
{
    switch (state) {
    case Entity_State::Alive: {
        vel += gravity * dt;
        pos += vel * dt;
        resolve_entity_collision();
        cooldown_weapon -= 1;

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

void Entity::jump(Vec2f gravity, Sample_Mixer *mixer)
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
            vel.y = gravity.y * -min(a, 0.6f);
            mixer->play_sample(jump_samples[rand() % 2]);
        } break;

        case Jump_State::Jump:
            break;
        }
    }
}

void spawn_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);

Entity entities[ENTITIES_COUNT];

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

void update_entities(Vec2f gravity, float dt)
{
    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        entities[i].update(gravity, dt);
    }
}

void render_entities(SDL_Renderer *renderer, Camera camera)
{
    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        entities[i].render(renderer, camera);
    }
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
