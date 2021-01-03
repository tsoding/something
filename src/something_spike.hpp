#ifndef SOMETHING_SPIKE_HPP_
#define SOMETHING_SPIKE_HPP_

enum class Spike_State
{
    Ded,
    Active,
    Poof,
};

struct Spike
{
    Spike_State state;
    Vec2f pos;
    float scale;
    Index<Texture> texture_index;
    float a;                     // 0.0..1.0

    void render(SDL_Renderer *renderer, Camera camera);
    void update(float dt);

    void activate();
};

Spike ice_spike(Vec2f pos);

struct Spike_Wave
{
    Vec2f pos;
    Vec2f dir;
    int count;
    float cooldown;

    void update(float dt, Game *game);
    void activate(Vec2f pos, Vec2f dir);
};

#endif  // SOMETHING_SPIKE_HPP_
