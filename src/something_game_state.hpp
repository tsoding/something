#ifndef SOMETHING_GAME_STATE_HPP_
#define SOMETHING_GAME_STATE_HPP_

enum class Debug_Draw_State {
    Idle = 0,
    Create,
    Delete
};

struct Entity_Index
{
    size_t unwrap;
};

struct Projectile_Index
{
    size_t unwrap;
};

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
    Frame_Animat active_animat;
    Frame_Animat poof_animat;
    float lifetime;
};

const size_t ENEMY_ENTITY_INDEX_OFFSET = 1;
const size_t PLAYER_ENTITY_INDEX = 0;

const size_t ENTITIES_COUNT = 69;
const size_t PROJECTILES_COUNT = 69;

struct Game_State
{
    Vec2f gravity;
    bool quit;
    bool debug;
    Vec2f collision_probe;
    Vec2f debug_mouse_position;
    Maybe<Projectile_Index> tracking_projectile;
    Debug_Draw_State state;
    Camera camera;
    Sample_Mixer mixer;

    Sprite ground_grass_texture;
    Sprite ground_texture;

    Frame_Animat entity_walking_animat;
    Frame_Animat entity_idle_animat;
    Sample_S16 entity_jump_sample1;
    Sample_S16 entity_jump_sample2;
    Sample_S16 player_shoot_sample;

    Frame_Animat projectile_active_animat;
    Frame_Animat projectile_poof_animat;

    TTF_Font *debug_font;

    Entity entities[ENTITIES_COUNT];
    Projectile projectiles[PROJECTILES_COUNT];

    // Whole Game State
    void update(float dt);
    void render(SDL_Renderer *renderer);
    void render_debug_overlay(SDL_Renderer *renderer);

    // Entities of the Game
    void reset_entities();
    void entity_shoot(Entity_Index entity_index);
    void entity_jump(Entity_Index entity_index);
    void inplace_spawn_entity(Entity_Index index, Vec2f pos);

    // Projectiles of the Game
    void spawn_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);
    int count_alive_projectiles(void);
    void render_projectiles(SDL_Renderer *renderer, Camera camera);
    void update_projectiles(float dt);
    Rectf hitbox_of_projectile(Projectile_Index index);
    Maybe<Projectile_Index> projectile_at_position(Vec2f position);
};

#endif  // SOMETHING_GAME_STATE_HPP_
