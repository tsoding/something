#include "something_color.hpp"
#include "something_particles.hpp"

void Particles::render(SDL_Renderer *renderer, Camera camera) const
{
    for (size_t i = 0; i < count; ++i) {
        const size_t j = (begin + i) % PARTICLES_CAPACITY;
        if (lifetimes[j] > 0.0f) {
            const Rectf particle = rect(
                positions[j] - vec2(sizes[j], sizes[j]) * 0.5f,
                sizes[j], sizes[j]);
            const auto opacity = lifetimes[j] / PARTICLE_LIFETIME;
            fill_rect(renderer, camera.to_screen(particle),
                      {colors[j].r, colors[j].g, colors[j].b, colors[j].a * opacity});
        }
    }
}


void Particles::push(float impact)
{
    if (count < PARTICLES_CAPACITY) {
        const size_t j = (begin + count) % PARTICLES_CAPACITY;
        positions[j] = source;
        velocities[j] = polar(impact, rand_float_range(PI, 2.0f * PI));
        lifetimes[j] = PARTICLE_LIFETIME;
        sizes[j] = rand_float_range(PARTICLE_SIZE_LOW, PARTICLE_SIZE_HIGH);
        // TODO(#187): implement HSL based generation of color for particles
        HSLA hsla = current_color;
        hsla.h += rand_float_range(0.0, 2.0 * PARTICLES_HUE_DEVIATION_DEGREE) - PARTICLES_HUE_DEVIATION_DEGREE;
        colors[j] = hsla.to_rgba();
        count += 1;
    }
}

void Particles::pop()
{
    assert(count > 0);
    begin = (begin + 1) % PARTICLES_CAPACITY;
    count -= 1;
}

void Particles::update(float dt, Tile_Grid *grid)
{

    for (size_t i = 0; i < count; ++i) {
        const size_t j = (begin + i) % PARTICLES_CAPACITY;
        lifetimes[j] -= dt;
        velocities[j] += vec2(0.0f, 1.0f) * PARTICLES_GRAVITY * dt;
        positions[j] += velocities[j] * dt;

        if (!grid->is_tile_empty_abs(positions[j])) {
            // lifetimes[j] = 0.0;
            velocities[j] = velocities[j] * -0.5f;
        }
    }

    while (count > 0 && lifetimes[begin] <= 0.0f) {
        pop();
    }

    cooldown -= dt;

    if (cooldown <= 0.0f && state == Particles::EMITTING) {
        push(rand_float_range(PARTICLE_VEL_LOW, PARTICLE_VEL_HIGH));
        const float PARTICLE_COOLDOWN = 1.0f / PARTICLES_RATE;
        cooldown = PARTICLE_COOLDOWN;
    }
}
