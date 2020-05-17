#ifndef SOMETHING_GAME_HPP_
#define SOMETHING_GAME_HPP_

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

struct Room_Index
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

    void kill();
};

const size_t ENEMY_ENTITY_INDEX_OFFSET = 1;
const size_t PLAYER_ENTITY_INDEX = 0;

const size_t ENTITIES_COUNT = 69;
const size_t PROJECTILES_COUNT = 69;
const size_t ROOM_ROW_COUNT = 8;

const size_t POPUP_BUFFER_CAPACITY = 256;

struct Popup
{
    char buffer[POPUP_BUFFER_CAPACITY];
    int buffer_size;
    TTF_Font *font;
    SDL_Color color;
    float a;

    void notify(SDL_Color color, const char *format, ...);
    void render(SDL_Renderer *renderer, const Camera *camera);
    void update(float delta_time);
};

struct Game
{
    bool quit;
    bool debug;
    Vec2f collision_probe;
    Vec2f debug_mouse_position;
    Maybe<Projectile_Index> tracking_projectile;
    Debug_Draw_State state;
    Camera camera;
    Sample_Mixer mixer;
    const Uint8 *keyboard;
    Popup popup;

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
    Room room_row[ROOM_ROW_COUNT];

    // Whole Game State
    void update(float dt);
    void render(SDL_Renderer *renderer);
    void render_debug_overlay(SDL_Renderer *renderer);

    // Entities of the Game
    void reset_entities();
    void entity_shoot(Entity_Index entity_index);
    void entity_jump(Entity_Index entity_index);
    void inplace_spawn_entity(Entity_Index index, Vec2f pos);
    void inplace_spawn_entity_from_file(Entity_Index index, Vec2f pos,
                                        const char *file_path);

    // Projectiles of the Game
    void spawn_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter);
    int count_alive_projectiles(void);
    void render_projectiles(SDL_Renderer *renderer, Camera camera);
    void update_projectiles(float dt);
    Rectf hitbox_of_projectile(Projectile_Index index);
    Maybe<Projectile_Index> projectile_at_position(Vec2f position);

    // Rooms of the Game
    Room_Index room_index_at(Vec2f p);
    void render_room_minimap(SDL_Renderer *renderer,
                             Room_Index index,
                             Vec2f position);
    void render_room_row_minimap(SDL_Renderer *renderer,
                                 Vec2f position);
    void render_entity_on_minimap(SDL_Renderer *renderer,
                                  Vec2f position,
                                  Vec2f entity_position);
};

#endif  // SOMETHING_GAME_HPP_
