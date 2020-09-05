#ifndef SOMETHING_GAME_HPP_
#define SOMETHING_GAME_HPP_

#include "something_console.hpp"
#include "something_particles.hpp"

enum Debug_Toolbar_Button
{
    DEBUG_TOOLBAR_TILES = 0,
    DEBUG_TOOLBAR_HEALS,
    DEBUG_TOOLBAR_ENEMIES,
    DEBUG_TOOLBAR_COUNT
};

enum class Debug_Draw_State {
    Idle = 0,
    Create,
    Delete
};

template <typename That>
struct Index
{
    size_t unwrap;

    bool operator==(const That that) const
    {
        return this->unwrap == that.unwrap;
    }

    bool operator!=(const That that) const
    {
        return !(*this == that);
    }
};

struct Entity_Index: public Index<Entity_Index> {};
struct Projectile_Index: public Index<Projectile_Index> {};

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

    void kill();
};

const size_t ENEMY_ENTITY_INDEX_OFFSET = 1;
const size_t PLAYER_ENTITY_INDEX = 0;

const size_t ENTITIES_COUNT = 69;
const size_t PROJECTILES_COUNT = 69;
const size_t ITEMS_COUNT = 69;
const size_t CAMERA_LOCKS_CAPACITY = 200;
const size_t ROOM_ROW_COUNT = 8;

struct Game
{
    bool quit;
    bool debug;
    bool step_debug;
    Vec2f collision_probe;
    Vec2f mouse_position;
    Vec2i original_mouse_position;
    Maybe<Projectile_Index> tracking_projectile;
    Debug_Draw_State state;
    Camera camera;
    Sample_Mixer mixer;
    const Uint8 *keyboard;
    Popup popup;
    // TODO(#178): disable game console in release mode
    Console console;

    Frame_Animat entity_walking_animat;
    Frame_Animat entity_idle_animat;
    Sample_S16 entity_jump_sample1;
    Sample_S16 entity_jump_sample2;
    Sample_S16 player_shoot_sample;
    Sample_S16 kill_enemy_sample;
    Sample_S16 damage_enemy_sample;

    Frame_Animat projectile_active_animat;
    Frame_Animat projectile_poof_animat;

    Bitmap_Font debug_font;
    Toolbar debug_toolbar;

    Entity entities[ENTITIES_COUNT];
    Projectile projectiles[PROJECTILES_COUNT];

    Item items[ITEMS_COUNT];

    Tile_Grid grid;

    Recti camera_locks[CAMERA_LOCKS_CAPACITY];
    size_t camera_locks_count;

    void add_camera_lock(Recti rect);

    // Whole Game State
    void update(float dt);
    void render(SDL_Renderer *renderer);
    void handle_event(SDL_Event *event);
    void render_debug_overlay(SDL_Renderer *renderer, size_t fps);

    // Entities of the Game
    void reset_entities();
    void entity_shoot(Entity_Index entity_index);
    void entity_jump(Entity_Index entity_index);
    void entity_resolve_collision(Entity_Index entity_index);
    void spawn_enemy_at(Vec2f pos);

    // Projectiles of the Game
    void spawn_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);
    int count_alive_projectiles(void);
    void render_projectiles(SDL_Renderer *renderer, Camera camera);
    void update_projectiles(float dt);
    Rectf hitbox_of_projectile(Projectile_Index index);
    Maybe<Projectile_Index> projectile_at_position(Vec2f position);

    // Items of the Game
    void spawn_health_at_mouse();
};

#endif  // SOMETHING_GAME_HPP_
