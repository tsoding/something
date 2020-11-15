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
    Frame_Animat_Index active_animat;
    Frame_Animat_Index poof_animat;
    float lifetime;

    void damage_tile(Tile *tile);
    void render(SDL_Renderer *renderer, Camera *camera);
    void update(float dt, Tile_Grid *grid);
    void kill();
};

Projectile rock_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);
Projectile water_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);
Projectile fire_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);
Projectile ice_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);

#endif  // SOMETHING_PROJECTILE_HPP_
