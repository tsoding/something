#ifndef SOMETHING_PROJECTILE_HPP_
#define SOMETHING_PROJECTILE_HPP_

enum class Projectile_State
{
    Ded = 0,
    Active,
    Poof
};

struct Projectile
{
    Entity_Index shooter;
    Projectile_State state;
    Vec2f pos;
    Vec2f vel;
    Frame_Animat_Index active_animat;
    Frame_Animat_Index poof_animat;
    float lifetime;

    void render(SDL_Renderer *renderer, Camera *camera);
    void update(float dt, Tile_Grid *grid);
    void kill();
};

Projectile default_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);

#endif  // SOMETHING_PROJECTILE_HPP_
