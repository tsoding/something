enum class Jump_State
{
    No_Jump = 0,
    Prepare,
    Jump
};

enum class Entity_State
{
    Ded = 0,
    Alive,
    Poof
};

enum class Alive_State
{
    Idle = 0,
    Walking
};

const size_t JUMP_SAMPLES_CAPACITY = 2;

struct Entity
{
    Entity_State state;
    Alive_State alive_state;
    Jump_State jump_state;

    Rectf texbox_local;
    Rectf hitbox_local;
    Vec2f pos;
    Vec2f vel;
    // TODO(#58): weapon cooldown should not be bound to framerate
    int cooldown_weapon;
    Vec2f gun_dir;

    Frame_Animat idle;
    Frame_Animat walking;
    Squash_Animat poof;
    Rubber_Animat prepare_for_jump_animat;
    Compose_Rubber_Animat<2> jump_animat;

    Sample_S16 jump_samples[JUMP_SAMPLES_CAPACITY];

    void resolve_entity_collision()
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

    void kill()
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
};

struct Entity_Index
{
    size_t unwrap;
};

void spawn_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);

const int ENTITY_COOLDOWN_WEAPON = 7;
const size_t ENTITIES_COUNT = 69;
Entity entities[ENTITIES_COUNT];

Rectf entity_texbox_world(const Entity entity)
{
    Rectf dstrect = {
        entity.texbox_local.x + entity.pos.x,
        entity.texbox_local.y + entity.pos.y,
        entity.texbox_local.w,
        entity.texbox_local.h
    };
    return dstrect;
}

Rectf entity_hitbox_world(const Entity entity)
{
    Rectf hitbox = {
        entity.hitbox_local.x + entity.pos.x, entity.hitbox_local.y + entity.pos.y,
        entity.hitbox_local.w, entity.hitbox_local.h
    };
    return hitbox;
}

void render_line(SDL_Renderer *renderer, Vec2f begin, Vec2f end)
{
    sec(SDL_RenderDrawLine(
            renderer,
            (int) floorf(begin.x), (int) floorf(begin.y),
            (int) floorf(end.x),   (int) floorf(end.y)));
}

void render_entity(SDL_Renderer *renderer, Camera camera, const Entity entity)
{
    assert(renderer);

    const SDL_RendererFlip flip = entity.gun_dir.x > 0.0f ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    switch (entity.state) {
    case Entity_State::Alive: {
        Rectf texbox = {};

        switch (entity.jump_state) {
        case Jump_State::No_Jump:
            texbox = entity_texbox_world(entity);
            break;

        case Jump_State::Prepare:
            texbox = entity.prepare_for_jump_animat.transform_rect(entity.texbox_local, entity.pos);
            break;

        case Jump_State::Jump:
            texbox = entity.jump_animat.transform_rect(entity.texbox_local, entity.pos);
            break;
        }

        switch (entity.alive_state) {
        case Alive_State::Idle: {
            render_animat(renderer, entity.idle, texbox - camera.pos, flip);
        } break;

        case Alive_State::Walking: {
            render_animat(renderer, entity.walking, texbox - camera.pos, flip);
        } break;
        }

        // TODO(#61): file with variables
        const float GUN_LENGTH = 50.0f;

        // TODO(#59): Proper gun rendering

        Vec2f gun_begin = entity.pos;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        render_line(
            renderer,
            gun_begin - camera.pos,
            gun_begin + normalize(entity.gun_dir) * GUN_LENGTH - camera.pos);
    } break;

    case Entity_State::Poof: {
        entity.poof.render(
            renderer,
            entity.pos - camera.pos,
            entity_texbox_world(entity) - camera.pos,
            entity.gun_dir.x > 0.0f ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
    } break;

    case Entity_State::Ded: {} break;
    }
}

void update_entity(Entity *entity, Vec2f gravity, float dt)
{
    assert(entity);

    switch (entity->state) {
    case Entity_State::Alive: {
        entity->vel += gravity * dt;
        entity->pos += entity->vel * dt;
        entity->resolve_entity_collision();
        entity->cooldown_weapon -= 1;

        switch (entity->jump_state) {
        case Jump_State::No_Jump:
            break;

        case Jump_State::Prepare:
            entity->prepare_for_jump_animat.update(dt);
            break;

        case Jump_State::Jump:
            entity->jump_animat.update(dt);
            if (entity->jump_animat.finished()) {
                entity->jump_state = Jump_State::No_Jump;
            }
            break;
        }

        switch (entity->alive_state) {
        case Alive_State::Idle: {
            update_animat(&entity->idle, dt);
        } break;

        case Alive_State::Walking: {
            update_animat(&entity->walking, dt);
        } break;
        }
    } break;

    case Entity_State::Poof: {
        entity->poof.update(dt);
        if (entity->poof.a >= 1) {
            entity->state = Entity_State::Ded;
        }
    } break;

    case Entity_State::Ded: {} break;
    }
}

void entity_shoot(Entity_Index entity_index)
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
}

void entity_point_gun_at(Entity_Index entity_index, Vec2f pos)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);
    Entity *entity = &entities[entity_index.unwrap];
    entity->gun_dir = normalize(pos - entity->pos);
}

void entity_jump(Entity_Index entity_index,
                 Vec2f gravity,
                 Sample_Mixer *mixer)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);
    Entity *entity = &entities[entity_index.unwrap];

    if (entity->state == Entity_State::Alive) {
        switch (entity->jump_state) {
        case Jump_State::No_Jump:
            entity->prepare_for_jump_animat.reset();
            entity->jump_state = Jump_State::Prepare;
            break;

        case Jump_State::Prepare: {
            float a = entity->prepare_for_jump_animat.t / entity->prepare_for_jump_animat.duration;
            entity->jump_animat.reset();
            entity->jump_state = Jump_State::Jump;
            entity->vel.y = gravity.y * -std::min(a, 0.6f);
            mixer->play_sample(entity->jump_samples[rand() % 2]);
        } break;

        case Jump_State::Jump:
            break;
        }
    }
}

void update_entities(Vec2f gravity, float dt)
{
    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        update_entity(&entities[i], gravity, dt);
    }
}

void render_entities(SDL_Renderer *renderer, Camera camera)
{
    for (size_t i = 0; i < ENTITIES_COUNT; ++i) {
        render_entity(renderer, camera, entities[i]);
    }
}

const int PLAYER_TEXBOX_SIZE = 64;
const int PLAYER_HITBOX_SIZE = PLAYER_TEXBOX_SIZE - 20;

void inplace_spawn_entity(Entity_Index index,
                          Frame_Animat walking, Frame_Animat idle,
                          Sample_S16 jump_sample1, Sample_S16 jump_sample2,
                          Vec2f pos = {0.0f, 0.0f})
{
    const Rectf texbox_local = {
        - (PLAYER_TEXBOX_SIZE / 2), - (PLAYER_TEXBOX_SIZE / 2),
        PLAYER_TEXBOX_SIZE, PLAYER_TEXBOX_SIZE
    };
    const Rectf hitbox_local = {
        - (PLAYER_HITBOX_SIZE / 2), - (PLAYER_HITBOX_SIZE / 2),
        PLAYER_HITBOX_SIZE, PLAYER_HITBOX_SIZE
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
