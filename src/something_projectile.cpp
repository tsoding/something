#include "./something_projectile.hpp"

const char *projectile_state_as_cstr(Projectile_State state)
{
    switch (state) {
    case Projectile_State::Ded: return "Ded";
    case Projectile_State::Active: return "Active";
    case Projectile_State::Poof: return "Poof";
    }

    assert(0 && "Incorrect Projectile_State");
    return "";
}

void Projectile::kill()
{
    if (state == Projectile_State::Active) {
        state = Projectile_State::Poof;
        assets.get_animat_by_index(poof_animat).reset();
    }
}

void Projectile::render(SDL_Renderer *renderer, Camera *camera)
{
    switch (state) {
    case Projectile_State::Active: {
        assets.get_animat_by_index(active_animat).render(
            renderer,
            camera->to_screen(pos));
    } break;

    case Projectile_State::Poof: {
        assets.get_animat_by_index(poof_animat).render(
            renderer,
            camera->to_screen(pos));
    } break;

    case Projectile_State::Ded: {} break;
    }
}

void Projectile::update(float dt, Tile_Grid *grid)
{
    switch (state) {
    case Projectile_State::Active: {
        assets.animats[active_animat.unwrap].unwrap.update(dt);
        pos += vel * dt;

        auto tile = grid->tile_at_abs(pos);
        if (tile && tile_defs[*tile].is_collidable) {
            kill();
            if ((TILE_DIRT_0 <= *tile && *tile < TILE_DIRT_3) ||
                (TILE_ICE_0 <= *tile && *tile < TILE_ICE_3)) {
                *tile += 1;
            } else if (*tile == TILE_DIRT_3 || *tile == TILE_ICE_3) {
                *tile = TILE_EMPTY;
            }
        }

        lifetime -= dt;

        if (lifetime <= 0.0f) {
            kill();
        }
    } break;

    case Projectile_State::Poof: {
        assets.animats[poof_animat.unwrap].unwrap.update(dt);
        if (assets.animats[poof_animat.unwrap].unwrap.frame_current ==
            (assets.animats[poof_animat.unwrap].unwrap.frame_count - 1)) {
            state = Projectile_State::Ded;
        }
    } break;

    case Projectile_State::Ded: {} break;
    }
}

Projectile default_projectile(Vec2f pos, Vec2f vel, Entity_Index shooter)
{
    Projectile result = {};
    result.state         = Projectile_State::Active;
    result.pos           = pos;
    result.vel           = vel;
    result.shooter       = shooter;
    result.lifetime      = PROJECTILE_LIFETIME;
    result.active_animat = PROJECTILE_IDLE_ANIMAT_INDEX;
    result.poof_animat   = PROJECTILE_POOF_ANIMAT_INDEX;
    return result;
}
