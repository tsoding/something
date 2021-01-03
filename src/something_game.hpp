#ifndef SOMETHING_GAME_HPP_
#define SOMETHING_GAME_HPP_

#include "something_console.hpp"
#include "something_particles.hpp"
#include "something_texture.hpp"
#include "something_background.hpp"
#include "something_projectile.hpp"
#include "something_spike.hpp"

enum Debug_Toolbar_Button
{
    DEBUG_TOOLBAR_TILES = 0,
    DEBUG_TOOLBAR_DESTROYABLE,
    DEBUG_TOOLBAR_HEALS,
    DEBUG_TOOLBAR_ENEMIES,
    DEBUG_TOOLBAR_DIRT,
    DEBUG_TOOLBAR_GOLEM,
    DEBUG_TOOLBAR_ICE_BLOCK,
    DEBUG_TOOLBAR_ICE_ITEM,
    DEBUG_TOOLBAR_ICE_GOLEM,
    DEBUG_TOOLBAR_COUNT
};

const size_t ENEMY_ENTITY_INDEX_OFFSET = 1;
const size_t PLAYER_ENTITY_INDEX = 0;

const size_t ENTITIES_COUNT = 69;
const size_t PROJECTILES_COUNT = 69;
const size_t ITEMS_COUNT = 69;
const size_t SPIKES_COUNT = 69;
const size_t CAMERA_LOCKS_CAPACITY = 200;
const size_t ROOM_ROW_COUNT = 8;
const size_t FPS_BARS_COUNT = 256;

struct Game
{
    bool quit;
    bool debug;
    bool step_debug;
    bool bfs_debug;
    bool fps_debug;
    bool holding_down_mouse;
    float frame_delays[FPS_BARS_COUNT];
    size_t frame_delays_begin;

    Vec2f collision_probe;
    Vec2f mouse_position;
    Maybe<Index<Projectile>> tracking_projectile;
    Camera camera;
    Sample_Mixer mixer;
    const Uint8 *keyboard;
    Popup popup;
    // TODO(#178): disable game console in release mode
    Console console;

    Index<Sample_S16> kill_enemy_sample;

    Bitmap_Font debug_font;
    Toolbar debug_toolbar;

    Entity entities[ENTITIES_COUNT];
    Projectile projectiles[PROJECTILES_COUNT];
    Spike spikes[SPIKES_COUNT];

    Item items[ITEMS_COUNT];

    Tile_Grid grid;

    Recti camera_locks[CAMERA_LOCKS_CAPACITY];
    size_t camera_locks_count;
    float camera_shaking_timeout;

    Background background;

    // TODO(#339): game supports only one spike wave
    //   We should implement a pool of them like entities, projectiles, spikes, etc.
    Spike_Wave spike_wave;

    // Camera utilities
    void add_camera_lock(Recti rect);
    void shake_camera(float duration);

    // Whole Game State
    void update(float dt);
    void render(SDL_Renderer *renderer);
    void handle_event(SDL_Event *event);
    void render_debug_overlay(SDL_Renderer *renderer, size_t fps);
    void render_fps_overlay(SDL_Renderer *renderer);
    void noclip(bool on);

    // Entities of the Game
    void reset_entities();
    void entity_shoot(Index<Entity> entity_index);
    void entity_jump(Index<Entity> entity_index);
    void entity_resolve_collision(Index<Entity> entity_index);
    void spawn_entity_at(Entity entity, Vec2f pos);
    void spawn_enemy_at(Vec2f pos);
    void spawn_golem_at(Vec2f pos);
    Vec2i where_entity_can_place_block(Index<Entity> index, bool *can_place = nullptr);
    bool does_tile_contain_entity(Vec2i tile_coord);
    void kill_entity(Entity *entity);
    void drop_all_items_of_entity(Entity *entity);
    void damage_entity(Entity *entity, int amount, Vec2f knockback);
    void damage_radius(Vec2f center, float radius, Index<Entity> stomper);

    // Spike
    void spawn_spike(Spike spike);

    // Projectiles of the Game
    void spawn_projectile(Projectile projectile);
    int count_alive_projectiles(void);
    void render_projectiles(SDL_Renderer *renderer, Camera camera);
    void update_projectiles(float dt);
    Rectf hitbox_of_projectile(Index<Projectile> index);
    Maybe<Index<Projectile>> projectile_at_position(Vec2f position);
    void projectile_collision(Projectile *a, Projectile *b);

    // Items of the Game
    void spawn_item_at(Item item, Vec2f pos);
    void spawn_health_at_mouse();
    void spawn_dirt_block_item_at(Vec2f pos);
    void spawn_dirt_block_item_at_mouse();
    int get_rooms_count(void);

    // Player related operations
    void render_player_hud(SDL_Renderer *renderer);
};

#endif  // SOMETHING_GAME_HPP_
