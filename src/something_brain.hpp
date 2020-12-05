#ifndef SOMETHING_BRAIN_HPP_
#define SOMETHING_BRAIN_HPP_

struct Brain
{
    void (*think)(Game *game, Entity_Index entity, Recti *lock);
};


void shooter_think(Game *game, Entity_Index entity, Recti *lock);
void stomper_think(Game *game, Entity_Index entity, Recti *lock);

Brain shooter_brain()
{
    return {shooter_think};
}

Brain stomper_brain()
{
    return {stomper_think};
}


#endif  // SOMETHING_BRAIN_HPP_
