#ifndef SOMETHING_ENTITY_H_
#define SOMETHING_ENTITY_H_

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
    Walking
};

const size_t JUMP_SAMPLES_CAPACITY = 2;

struct Entity
{
    Entity_State state;
    Alive_State alive_state;
    Jump_State jump_state;

    Rectf texbox_local;
    Rectf hitbox_local;
    Vec2f pos;
    Vec2f vel;
    float cooldown_weapon;
    Vec2f gun_dir;
    int lives;

    Frame_Animat idle;
    Frame_Animat walking;
    Squash_Animat poof;
    Rubber_Animat prepare_for_jump_animat;
    Compose_Rubber_Animat<2> jump_animat;

    Sample_S16 jump_samples[JUMP_SAMPLES_CAPACITY];
    Sample_S16 shoot_sample;

    void resolve_entity_collision(Room *room_row, size_t room_row_count);
    void kill();

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

    void render(SDL_Renderer *renderer, Camera camera) const;
    void update(float dt, Room *room_row, size_t room_row_count);
    void point_gun_at(Vec2f target);
    void jump(Sample_Mixer *mixer);
};


#endif  // SOMETHING_ENTITY_H_
