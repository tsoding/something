enum class Entity_Dir
{
    Right = 0,
    Left
};

enum class Entity_State
{
    Ded = 0,
    Alive
};

struct Entity
{
    Entity_State state;

    Rectf texbox_local;
    Rectf hitbox_local;
    Vec2f pos;
    Vec2f vel;

    Animat idle;
    Animat walking;
    Animat *current;
    Entity_Dir dir;

    int cooldown_weapon;
};

void resolve_point_collision(Vec2f *p)
{
    assert(p);

    const auto tile = vec_cast<int>(*p / (float) TILE_SIZE);

    if (is_tile_empty(tile)) {
        return;
    }

    const auto p0 = vec_cast<float>(tile * TILE_SIZE);
    const auto p1 = vec_cast<float>((tile + 1) * TILE_SIZE);

    struct Side {
        float d;
        Vec2f np;
        Vec2<int> nd;
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

    if (entity.state == Entity_State::Ded) return;

    const SDL_RendererFlip flip =
        entity.dir == Entity_Dir::Right
        ? SDL_FLIP_NONE
        : SDL_FLIP_HORIZONTAL;
    render_animat(renderer, *entity.current, entity_texbox_world(entity) - camera.pos, flip);
}

void update_entity(Entity *entity, Vec2f gravity, float dt)
{
    assert(entity);

    if (entity->state == Entity_State::Ded) return;

    entity->vel += gravity * dt;
    entity->pos += entity->vel * dt;
    resolve_entity_collision(entity);

    entity->cooldown_weapon -= 1;

    update_animat(entity->current, dt);
}

void entity_move(Entity *entity, float speed)
{
    assert(entity);

    entity->vel.x = speed;

    if (speed < 0) {
        entity->dir = Entity_Dir::Left;
    } else if (speed > 0) {
        entity->dir = Entity_Dir::Right;
    }

    entity->current = &entity->walking;
}

void entity_stop(Entity *entity)
{
    assert(entity);
    entity->vel.x = 0;
    entity->current = &entity->idle;
}

const int ENTITY_COOLDOWN_WEAPON = 7;
const int ENTITIES_COUNT = 69;
Entity entities[ENTITIES_COUNT];
// TODO: introduce a int typedef that indicates Entity Id

void entity_shoot(int entity_index)
{
    assert(0 <= entity_index);
    assert(entity_index < ENTITIES_COUNT);

    Entity *entity = &entities[entity_index];

    if (entity->state == Entity_State::Ded) return;
    if (entity->cooldown_weapon > 0) return;

    if (entity->dir == Entity_Dir::Right) {
        spawn_projectile(entity->pos, vec2(10.0f, 0.0f), entity_index);
    } else {
        spawn_projectile(entity->pos, vec2(-10.0f, 0.0f), entity_index);
    }

    entity->cooldown_weapon = ENTITY_COOLDOWN_WEAPON;
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
