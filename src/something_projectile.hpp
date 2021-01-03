#ifndef SOMETHING_PROJECTILE_HPP_
#define SOMETHING_PROJECTILE_HPP_

struct Game;
struct Entity;

enum class Projectile_State
{
    Ded = 0,
    Active,
    Poof
};

enum class Tile_Damage
{
    None,
    Dirt,
    Ice,
};

enum class Projectile_Kind
{
    None,
    Fire,
    Ice,
    Rock,
    Water,
};

struct Projectile
{
    Projectile_Kind kind;
    Tile_Damage tile_damage;
    Index<Entity> shooter;
    Projectile_State state;
    Vec2f pos;
    Vec2f vel;
    Frames_Animat active_animat;
    Frames_Animat poof_animat;
    float lifetime;

    void damage_tile(Tile *tile);
    void render(SDL_Renderer *renderer, Camera *camera);
    void update(float dt, Tile_Grid *grid);
    void kill();
    Rectf hitbox();
};

Projectile rock_projectile(Vec2f pos, Vec2f vel, Index<Entity> shooter);
Projectile water_projectile(Vec2f pos, Vec2f vel, Index<Entity> shooter);
Projectile fire_projectile(Vec2f pos, Vec2f vel, Index<Entity> shooter);
Projectile ice_projectile(Vec2f pos, Vec2f vel, Index<Entity> shooter);

#endif  // SOMETHING_PROJECTILE_HPP_
