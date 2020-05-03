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
    Debug_Draw_State state;
    Camera camera;

    Sprite ground_grass_texture;
    Sprite ground_texture;

    TTF_Font *debug_font;

    Maybe<Projectile_Index> tracking_projectile;

    Entity entities[ENTITIES_COUNT];
    Projectile projectiles[PROJECTILES_COUNT];
    // Whole Game State
    void update(float dt);
    void render(SDL_Renderer *renderer);
    void render_debug_overlay(SDL_Renderer *renderer);

    // Entities of the Game
    void reset_entities(Frame_Animat walking, Frame_Animat idle,
                        Sample_S16 jump_sample1, Sample_S16 jump_sample2);
    void entity_shoot(Entity_Index entity_index);
    void inplace_spawn_entity(Entity_Index index,
                              Frame_Animat walking,
                              Frame_Animat idle,
                              Sample_S16 jump_sample1,
                              Sample_S16 jump_sample2,
                              Vec2f pos);

    // Projectiles of the Game
    void init_projectiles(Frame_Animat active_animat, Frame_Animat poof_animat);
    void spawn_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);
    int count_alive_projectiles(void);
    void render_projectiles(SDL_Renderer *renderer, Camera camera);
    void update_projectiles(float dt);
    Rectf hitbox_of_projectile(Projectile_Index index);
    Maybe<Projectile_Index> projectile_at_position(Vec2f position);
};

#endif  // SOMETHING_GAME_STATE_HPP_
