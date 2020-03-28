enum class Entity_Dir
{
    Right = 0,
    Left
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

struct Entity
{
    Entity_State state;
    Alive_State alive_state;

    Rectf texbox_local;
    Rectf hitbox_local;
    Vec2f pos;
    Vec2f vel;

    Frame_Animat idle;
    Frame_Animat walking;
    Squash_Animat poof;

    Entity_Dir dir;

    int cooldown_weapon;

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
};

struct Entity_Index
{
    size_t unwrap;
};

void spawn_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);

const int ENTITY_COOLDOWN_WEAPON = 7;
const size_t ENTITIES_COUNT = 69;
Entity entities[ENTITIES_COUNT];
// TODO(#36): introduce a typedef that indicates Entity Id

Sprite last_alive_frame_of_entity(Entity_Index entity_index)
{
    Entity entity = entities[entity_index.unwrap];

    assert(entity.state == Entity_State::Alive);

    switch (entity.alive_state) {
    case Alive_State::Idle:
        assert(entity.idle.frame_count > 0);
        return entity.idle.frames[entity.idle.frame_current];
    case Alive_State::Walking:
        assert(entity.walking.frame_count > 0);
        return entity.walking.frames[entity.walking.frame_current];
    }

    assert(0 && "Incorrent Alive_State value");

    return {};
}

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

void render_entity(SDL_Renderer *renderer, Camera camera, const Entity entity)
{
    assert(renderer);

    const SDL_RendererFlip flip =
        entity.dir == Entity_Dir::Right
        ? SDL_FLIP_NONE
        : SDL_FLIP_HORIZONTAL;

    switch (entity.state) {
    case Entity_State::Alive: {
        switch (entity.alive_state) {
        case Alive_State::Idle: {
            render_animat(renderer, entity.idle, entity_texbox_world(entity) - camera.pos, flip);
        } break;

        case Alive_State::Walking: {
            render_animat(renderer, entity.walking, entity_texbox_world(entity) - camera.pos, flip);
        } break;
        }
    } break;

    case Entity_State::Poof: {
        entity.poof.render(
            renderer,
            entity.pos - camera.pos,
            entity_texbox_world(entity) - camera.pos,
            entity.dir == Entity_Dir::Right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
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

void entity_move(Entity_Index entity_index, float speed)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);

    Entity *entity = entities + entity_index.unwrap;

    if (entity->state == Entity_State::Alive) {
        entity->vel.x = speed;

        if (speed < 0) {
            entity->dir = Entity_Dir::Left;
        } else if (speed > 0) {
            entity->dir = Entity_Dir::Right;
        }

        entity->alive_state = Alive_State::Walking;
    }
}

void entity_stop(Entity_Index entity_index)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);

    Entity *entity = entities + entity_index.unwrap;

    if (entity->state == Entity_State::Alive) {
        entity->vel.x = 0;
        entity->alive_state = Alive_State::Idle;
    }
}

void entity_shoot(Entity_Index entity_index)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);

    Entity *entity = &entities[entity_index.unwrap];

    if (entity->state != Entity_State::Alive) return;
    if (entity->cooldown_weapon > 0) return;

    if (entity->dir == Entity_Dir::Right) {
        spawn_projectile(entity->pos, vec2(10.0f, 0.0f), entity_index);
    } else {
        spawn_projectile(entity->pos, vec2(-10.0f, 0.0f), entity_index);
    }

    entity->cooldown_weapon = ENTITY_COOLDOWN_WEAPON;
}

void kill_entity(Entity_Index entity_index)
{
    assert(entity_index.unwrap < ENTITIES_COUNT);

    Entity *entity = &entities[entity_index.unwrap];

    // TODO(#40): entity poof animation should use the last alive frame
    if (entity->state == Entity_State::Alive) {
        entity->poof.a = 0.0f;
        entity->poof.sprite = last_alive_frame_of_entity(entity_index);
        entity->state = Entity_State::Poof;
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
