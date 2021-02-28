#ifndef SOMETHING_BRAIN_HPP_
#define SOMETHING_BRAIN_HPP_

struct Brain
{
    void (*think)(Game *game, Index<Entity> entity, Recti *lock);
};


void shooter_think(Game *game, Index<Entity> entity, Recti *lock);
void stomper_think(Game *game, Index<Entity> entity, Recti *lock);
void follower_think(Game *game, Index<Entity> entity_index, Recti *lock);
void shooter_stomper_think(Game *game, Index<Entity> entity_index, Recti *lock);

Brain shooter_brain()
{
    return {shooter_think};
}

Brain stomper_brain()
{
    return {stomper_think};
}

Brain follower_brain()
{
    return {follower_think};
}

Brain shooter_stomper_brain()
{
    return {shooter_stomper_think};
}

#endif  // SOMETHING_BRAIN_HPP_
