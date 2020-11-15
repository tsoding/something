#ifndef SOMETHING_WEAPON_HPP_
#define SOMETHING_WEAPON_HPP_

#include "./something_projectile.hpp"

struct Game;

struct Gun
{
    Projectile projectile;
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
};

struct Weapon2
{
    Weapon_Type type;
    Gun gun;
    Placer placer;

    void render(SDL_Renderer *renderer, Game *game, Entity_Index entity);
    void shoot(Game *game, Entity_Index shooter);
    Sprite icon() const;
};

Weapon2 water_gun();
Weapon2 fire_gun();
Weapon2 rock_gun();
Weapon2 ice_gun();
Weapon2 dirt_block_placer();
Weapon2 ice_block_placer();

#endif  // SOMETHING_WEAPON_HPP_
