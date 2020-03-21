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
};

void resolve_point_collision(Vec2f *p)
{
    assert(p);

    const auto tile = vec_cast<int>(*p / TILE_SIZE);

    if (is_tile_empty(tile)) {
        return;
    }

    const auto p0 = vec_cast<float>(tile) * TILE_SIZE;
    const auto p1 = vec_cast<float>(tile + 1) * TILE_SIZE;

    struct Side {
        float d;
        Vec2f np;
        Vec2i nd;
        float dd;
    };

    Side sides[] = {
        {sqr_dist<float>({p0.x, 0},    {p->x, 0}),    {p0.x, p->y}, {-1,  0}, TILE_SIZE_SQR},     // left
        {sqr_dist<float>({p1.x, 0},    {p->x, 0}),    {p1.x, p->y}, { 1,  0}, TILE_SIZE_SQR},     // right
        {sqr_dist<float>({0, p0.y},    {0, p->y}),    {p->x, p0.y}, { 0, -1}, TILE_SIZE_SQR},     // top
        {sqr_dist<float>({0, p1.y},    {0, p->y}),    {p->x, p1.y}, { 0,  1}, TILE_SIZE_SQR},     // bottom
        {sqr_dist<float>({p0.x, p0.y}, {p->x, p->y}), {p0.x, p0.y}, {-1, -1}, TILE_SIZE_SQR * 2}, // top-left
        {sqr_dist<float>({p1.x, p0.y}, {p->x, p->y}), {p1.x, p0.y}, { 1, -1}, TILE_SIZE_SQR * 2}, // top-right
        {sqr_dist<float>({p0.x, p1.y}, {p->x, p->y}), {p0.x, p1.y}, {-1,  1}, TILE_SIZE_SQR * 2}, // bottom-left
        {sqr_dist<float>({p1.x, p1.y}, {p->x, p->y}), {p1.x, p1.y}, { 1,  1}, TILE_SIZE_SQR * 2}  // bottom-right
    };
    const int SIDES_COUNT = sizeof(sides) / sizeof(sides[0]);

    int closest = -1;
    for (int current = 0; current < SIDES_COUNT; ++current) {
        for (int i = 1;
             !is_tile_empty(tile + (sides[current].nd * i)) ;
             ++i)
        {
            sides[current].d += sides[current].dd;
        }

        if (closest < 0 || sides[closest].d >= sides[current].d) {
            closest = current;
        }
    }

    *p = sides[closest].np;
}

void resolve_entity_collision(Entity *entity)
{
    assert(entity);

    Vec2f p0 = vec2(entity->hitbox_local.x, entity->hitbox_local.y) + entity->pos;
    Vec2f p1 = p0 + vec2(entity->hitbox_local.w, entity->hitbox_local.h);

    Vec2f mesh[] = {
        p0,
        {p1.x, p0.y},
        {p0.x, p1.y},
        p1,
    };
    const int MESH_COUNT = sizeof(mesh) / sizeof(mesh[0]);

    for (int i = 0; i < MESH_COUNT; ++i) {
        Vec2f t = mesh[i];
        resolve_point_collision(&t);
        Vec2f d = t - mesh[i];

        const int IMPACT_THRESHOLD = 5;
        if (abs(d.y) >= IMPACT_THRESHOLD) entity->vel.y = 0;
        if (abs(d.x) >= IMPACT_THRESHOLD) entity->vel.x = 0;

        for (int j = 0; j < MESH_COUNT; ++j) {
            mesh[j] += d;
        }

        entity->pos += d;
    }
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
        entity.poof.render(renderer,
                           entity.pos - camera.pos,
                           entity_texbox_world(entity) - camera.pos);
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
        resolve_entity_collision(entity);
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

void entity_move(Entity *entity, float speed)
{
    assert(entity);

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

void entity_stop(Entity *entity)
{
    assert(entity);

    if (entity->state == Entity_State::Alive) {
        entity->vel.x = 0;
        entity->alive_state = Alive_State::Idle;
    }
}

const int ENTITY_COOLDOWN_WEAPON = 7;
const int ENTITIES_COUNT = 69;
Entity entities[ENTITIES_COUNT];
// TODO(#36): introduce a typedef that indicates Entity Id

void entity_shoot(int entity_index)
{
    assert(0 <= entity_index);
    assert(entity_index < ENTITIES_COUNT);

    Entity *entity = &entities[entity_index];

    if (entity->state != Entity_State::Alive) return;
    if (entity->cooldown_weapon > 0) return;

    if (entity->dir == Entity_Dir::Right) {
        spawn_projectile(entity->pos, vec2(10.0f, 0.0f), entity_index);
    } else {
        spawn_projectile(entity->pos, vec2(-10.0f, 0.0f), entity_index);
    }

    entity->cooldown_weapon = ENTITY_COOLDOWN_WEAPON;
}

void kill_entity(int entity_index)
{
    Entity *entity = &entities[entity_index];

    // TODO(#40): entity poof animation should use the last alive frame
    if (entity->state == Entity_State::Alive) {
        entity->state = Entity_State::Poof;
        entity->poof.a = 0.0f;
        assert(entity->idle.frame_count > 0);
        entity->poof.sprite = entity->idle.frames[0];
    }
}

void update_entities(Vec2f gravity, float dt)
{
    for (int i = 0; i < ENTITIES_COUNT; ++i) {
        update_entity(&entities[i], gravity, dt);
    }
}

void render_entities(SDL_Renderer *renderer, Camera camera)
{
    for (int i = 0; i < ENTITIES_COUNT; ++i) {
        render_entity(renderer, camera, entities[i]);
    }
}
