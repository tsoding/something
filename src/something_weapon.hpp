#ifndef SOMETHING_WEAPON_HPP_
#define SOMETHING_WEAPON_HPP_

#include "./something_projectile.hpp"

struct Game;

// TODO(#356): gun does not have a recoil animation
struct Gun
{
    Projectile projectile;
    Index<Texture> skin;
    float skin_scale;
};

struct Placer
{
    Tile tile;
    int amount;
};

enum class Weapon_Type
{
    Gun,
    Placer,
    Stomp,
};

struct Weapon
{
    Weapon_Type type;
    Gun gun;
    Placer placer;

    Maybe<Index<Sample_S16>> shoot_sample;

    void render(SDL_Renderer *renderer, Game *game, Index<Entity> entity, RGBA shade = {});
    void shoot(Game *game, Index<Entity> shooter);
    Sprite icon() const;
};

Weapon water_gun();
Weapon fire_gun();
Weapon rock_gun();
Weapon ice_gun();
Weapon dirt_block_placer(int amount = 0);
Weapon ice_block_placer(int amount = 0);
Weapon stomp_move();


#endif  // SOMETHING_WEAPON_HPP_
