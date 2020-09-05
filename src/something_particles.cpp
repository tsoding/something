#include "something_particles.hpp"

void Particles::render(SDL_Renderer *renderer, Camera camera) const
{
    for (size_t i = 0; i < count; ++i) {
        const size_t j = (begin + i) % PARTICLES_CAPACITY;
        const Rectf particle = rect(
            positions[j] - vec2(sizes[j], sizes[j]) * 0.5f,
            sizes[j], sizes[j]);
        const auto opacity = lifetimes[j] / PARTICLE_LIFETIME;
        fill_rect(renderer, camera.to_screen(particle),
                  {colors[j].r, colors[j].g, colors[j].b, (Uint8) (colors[j].a * opacity)});
    }
}

Vec2f polar(float mag, float angle)
{
    return vec2(cosf(angle), sinf(angle)) * mag;
}

float rand_float_range(float low, float high)
{
    const auto r = (float)rand()/(float)(RAND_MAX);
    return low + r * (high - low);
}

void Particles::push(Vec2f source)
{
    if (count < PARTICLES_CAPACITY && state == Particles::EMITTING) {
        const size_t j = (begin + count) % PARTICLES_CAPACITY;
        positions[j] = source;
        velocities[j] = polar(
            rand_float_range(PARTICLE_VEL_LOW, PARTICLE_VEL_HIGH),
            rand_float_range(0.0f, 2.0f * PI));
        lifetimes[j] = PARTICLE_LIFETIME;
        sizes[j] = rand_float_range(PARTICLE_SIZE_LOW, PARTICLE_SIZE_HIGH);
        // TODO: implement HSL based generation of color for particles
        colors[j] = {
            (Uint8) (rand() % 255),
            (Uint8) (rand() % 255),
            (Uint8) (rand() % 255),
            255
        };
        count += 1;
    }
}

void Particles::pop()
{
    assert(count > 0);
    begin = (begin + 1) % PARTICLES_CAPACITY;
    count -= 1;
}

void Particles::update(float dt, Vec2f source)
{
    for (size_t i = 0; i < count; ++i) {
        const size_t j = (begin + i) % PARTICLES_CAPACITY;
        lifetimes[j] -= dt;
        positions[j] += velocities[j] * dt;
    }

    while (count > 0 && lifetimes[begin] <= 0.0f) {
        pop();
    }

    cooldown -= dt;

    if (cooldown <= 0.0f) {
        push(source);
        const float PARTICLE_COOLDOWN = 1.0f / PARTICLES_RATE;
        cooldown = PARTICLE_COOLDOWN;
    }
}
