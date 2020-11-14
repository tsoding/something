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
