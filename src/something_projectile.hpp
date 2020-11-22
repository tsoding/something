#ifndef SOMETHING_PROJECTILE_HPP_
#define SOMETHING_PROJECTILE_HPP_

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

struct Projectile
{
    Tile_Damage tile_damage;
    Entity_Index shooter;
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
    void collide_with(Projectile *other);
    Rectf hitbox();
};

Projectile rock_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);
Projectile water_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);
Projectile fire_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);
Projectile ice_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);

#endif  // SOMETHING_PROJECTILE_HPP_
