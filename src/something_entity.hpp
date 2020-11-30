#ifndef SOMETHING_ENTITY_H_
#define SOMETHING_ENTITY_H_

#include "./something_particles.hpp"
#include "./something_weapon.hpp"
#include "./something_item.hpp"

enum class Jump_State
{
    No_Jump = 0,
    Prepare,
    Jump
};

enum class Entity_State
{
    Ded = 0,
    Alive,
    Poof
};

enum class Alive_State
{
    Idle = 0,
    Walking,
    Stomping,
    Unstomping,
};

const char *alive_state_as_cstr(Alive_State state);

const size_t JUMP_SAMPLES_CAPACITY = 2;

const size_t WEAPON_SLOTS_CAPACITY = 10;
const size_t ITEMS_CAPACITY = 10;

struct Entity
{
    enum Direction
    {
        Right,
        Left
    };

    Entity_State state;
    Alive_State alive_state;
    Jump_State jump_state;
    // NOTE: indicates that the entity jump_state has transitioned
    // from Jump_State::Prepare to Jump_State::Jump and the force that
    // drives the entity up has been applied. That flag is used by the
    // collision system to know when to not cancel out the vertical
    // velocity which can accidentally cancel out the whole jump.
    bool has_jumped;
    bool noclip;

    Rectf texbox_local;
    Rectf hitbox_local;
    Vec2f pos;
    Vec2f vel;
    float speed;
    float cooldown_weapon;
    Vec2f gun_dir;
    int lives;
    RGBA flash_color;
    float flash_alpha;
    Direction walking_direction;

    Weapon weapon_slots[WEAPON_SLOTS_CAPACITY];
    size_t weapon_slots_count;
    size_t weapon_current;

    struct Item_Slot
    {
        Item item;
        size_t count;
    };

    Item_Slot items[ITEMS_CAPACITY];
    size_t items_count;

    Frames_Animat idle;
    Frames_Animat walking;
    Rubber_Animat poof_animat;
    Rubber_Animat prepare_for_jump_animat;
    Compose_Rubber_Animat<2> jump_animat;
    Rubber_Animat unstomp_animat;

    Sample_S16_Index jump_samples[JUMP_SAMPLES_CAPACITY];

    int count_jumps;
    int max_allowed_jumps;

    Particles particles;

    inline Rectf texbox_world() const
    {
        Rectf dstrect = {
            texbox_local.x + pos.x,
            texbox_local.y + pos.y,
            texbox_local.w,
            texbox_local.h
        };
        return dstrect;
    }

    inline Rectf hitbox_world() const
    {
        Rectf hitbox = {
            hitbox_local.x + pos.x, hitbox_local.y + pos.y,
            hitbox_local.w, hitbox_local.h
        };
        return hitbox;
    }

    void jump();
    void move(Direction direction);
    void stop();
    void stomp(Tile_Grid *grid);

    void render(SDL_Renderer *renderer, Camera camera,
                RGBA shade = {0, 0, 0, 0}) const;
    void render_debug(SDL_Renderer *renderer, Camera camera, Bitmap_Font *font) const;
    void update(float dt, Game *game, Entity_Index me);
    void point_gun_at(Vec2f target);
    void flash(RGBA color);
    Vec2f feet();
    bool ground(Tile_Grid *grid);
    Weapon *get_current_weapon();
    void push_weapon(Weapon weapon);
    void push_item(Item item, size_t count = 1);
};

Entity player_entity(Vec2f pos);
Entity enemy_entity(Vec2f pos);
Entity golem_entity(Vec2f pos);
Entity ice_golem_entity(Vec2f pos);

#endif  // SOMETHING_ENTITY_H_
