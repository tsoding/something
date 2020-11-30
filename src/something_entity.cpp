#include "./something_print.hpp"
#include "./something_entity.hpp"

const char *alive_state_as_cstr(Alive_State state)
{
    switch (state) {
    case Alive_State::Idle:     return "Idle";
    case Alive_State::Walking:  return "Walking";
    case Alive_State::Stomping: return "Stomping";
    }
    assert(0 && "unreachable");
}

void Entity::push_item(Item item, size_t count)
{
    assert(items_count < ITEMS_CAPACITY);
    items[items_count++] = {item, count};
}

RGBA mix_colors(RGBA b32, RGBA a32)
{
    const float r_alpha = a32.a + b32.a * (1.0f - a32.a);

    RGBA r = {};

    r.r = (a32.r * a32.a + b32.r * b32.a * (1.0f - a32.a)) / r_alpha;
    r.g = (a32.g * a32.a + b32.g * b32.a * (1.0f - a32.a)) / r_alpha;
    r.b = (a32.b * a32.a + b32.b * b32.a * (1.0f - a32.a)) / r_alpha;
    r.a = r_alpha;

    return r;
}

void Entity::render(SDL_Renderer *renderer, Camera camera, RGBA shade) const
{
    const SDL_RendererFlip flip =
        gun_dir.x > 0.0f ?
        SDL_FLIP_NONE :
        SDL_FLIP_HORIZONTAL;

    // TODO(#185): should we use shade for the particles of an entity?
    particles.render(renderer, camera);

    switch (state) {
    case Entity_State::Alive: {
        // Figuring out texbox
        Rectf texbox = {};

        switch (jump_state) {
        case Jump_State::No_Jump:
            texbox = texbox_world();
            break;

        case Jump_State::Prepare:
            texbox = prepare_for_jump_animat.transform_rect(texbox_local, pos);
            break;

        case Jump_State::Jump:
            texbox = jump_animat.transform_rect(texbox_local, pos);
            break;
        }

        // Rendering Live Bar
        {
            const Rectf livebar_border = {
                texbox.x + texbox.w * 0.5f - ENTITY_LIVEBAR_WIDTH * 0.5f,
                texbox.y - ENTITY_LIVEBAR_HEIGHT - ENTITY_LIVEBAR_PADDING_BOTTOM,
                ENTITY_LIVEBAR_WIDTH,
                ENTITY_LIVEBAR_HEIGHT
            };
            const float percent = (float) lives / (float) ENTITY_MAX_LIVES;
            const Rectf livebar_remain = {
                livebar_border.x, livebar_border.y,
                ENTITY_LIVEBAR_WIDTH * percent,
                ENTITY_LIVEBAR_HEIGHT
            };
            RGBA entity_livebar_color;
            if (percent > 0.75f) {
                entity_livebar_color = ENTITY_LIVEBAR_FULL_COLOR;
            } else if (0.25f < percent && percent < 0.75f) {
                entity_livebar_color = ENTITY_LIVEBAR_HALF_COLOR;
            } else {
                entity_livebar_color = ENTITY_LIVEBAR_LOW_COLOR;
            }
            SDL_Color sdl_entity_livebar_color = rgba_to_sdl(mix_colors(entity_livebar_color, shade));
            sec(SDL_SetRenderDrawColor(
                        renderer,
                        sdl_entity_livebar_color.r,
                        sdl_entity_livebar_color.g,
                        sdl_entity_livebar_color.b,
                        sdl_entity_livebar_color.a));
            const auto rect_border = rectf_for_sdl(camera.to_screen(livebar_border));
            sec(SDL_RenderDrawRect(renderer, &rect_border));
            const auto rect_remain = rectf_for_sdl(camera.to_screen(livebar_remain));
            sec(SDL_RenderFillRect(renderer, &rect_remain));
        }

        RGBA effective_flash_color = flash_color;
        effective_flash_color.a = flash_alpha;

        // Render the character
        switch (alive_state) {
        case Alive_State::Idle: {
            idle.render(
                renderer, camera.to_screen(texbox), flip,
                mix_colors(shade, effective_flash_color));
        } break;

        case Alive_State::Walking: {
            walking.render(
                renderer, camera.to_screen(texbox), flip,
                mix_colors(shade, effective_flash_color));
        } break;

        case Alive_State::Stomping: {
            idle.render(
                renderer, camera.to_screen(texbox), flip,
                mix_colors(shade, effective_flash_color));
        } break;
        }

        // Render the gun
        // TODO(#59): Proper gun rendering
        Vec2f gun_begin = pos;
        render_line(
            renderer,
            camera.to_screen(gun_begin),
            camera.to_screen(gun_begin + normalize(gun_dir) * ENTITY_GUN_LENGTH),
            {1.0f, 0.0f, 0.0f, 1.0f});
    } break;

    case Entity_State::Poof: {
        Rectf texbox = poof_animat.transform_rect(texbox_local, pos);
        // TODO(#151): Poof state loses last alive frame
        //   Previous animation implementation was capturing texture of last alive state.
        //   So if entity was shot in running pose it was squashing in this position.
        //   So there's no sudden graphical switch to idle texture.
        idle.render(renderer, camera.to_screen(texbox), flip, shade);
    } break;

    case Entity_State::Ded: {} break;
    }
}

void Entity::render_debug(SDL_Renderer *renderer, Camera camera, Bitmap_Font *font) const
{
    if (state == Entity_State::Alive) {
        // Collision mesh
        const float step_x = hitbox_local.w / (float) ENTITY_MESH_COLS;
        const float step_y = hitbox_local.h / (float) ENTITY_MESH_ROWS;

        for (int rows = 0; rows <= ENTITY_MESH_ROWS; ++rows) {
            for (int cols = 0; cols <= ENTITY_MESH_COLS; ++cols) {
                Vec2f t = camera.to_screen(
                    pos +
                    vec2(hitbox_local.x, hitbox_local.y) +
                    vec2(cols * step_x, rows * step_y));
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                const int PROBE_SIZE = 10;
                SDL_Rect rect = {
                    (int)t.x - PROBE_SIZE / 2,
                    (int)t.y - PROBE_SIZE / 2,
                    PROBE_SIZE,
                    PROBE_SIZE
                };
                sec(SDL_RenderFillRect(renderer, &rect));
            }
        }

        // Alive state
        auto tbw = texbox_world();

        font->render(
            renderer,
            camera.to_screen(vec2(tbw.x, tbw.y + tbw.h)),
            {FONT_DEBUG_SIZE, FONT_DEBUG_SIZE},
            FONT_DEBUG_COLOR,
            alive_state_as_cstr(alive_state));
    }
}

HSLA get_particle_color_for_tile(Tile_Grid *grid, Vec2f pos)
{
    const auto tile_sprite = tile_defs[*grid->tile_at_abs(pos + vec2(0.0f, TILE_SIZE * 0.5f))].top_texture;
    const auto surface = assets.get_texture_by_index(tile_sprite.texture_index).surface;
    const auto x = rand() % tile_sprite.srcrect.w;
    sec(SDL_LockSurface(surface));
    HSLA result = {};
    {
        assert(surface->format->format == SDL_PIXELFORMAT_RGBA32);
        const auto pixel = *(Uint32*) ((uint8_t *) surface->pixels + tile_sprite.srcrect.y * surface->pitch + (tile_sprite.srcrect.x + x) * sizeof(Uint32));
        SDL_Color color = {};
        SDL_GetRGBA(
            pixel,
            surface->format,
            &color.r,
            &color.g,
            &color.b,
            &color.a);
        result = sdl_to_rgba(color).to_hsla();
    }
    SDL_UnlockSurface(surface);
    return result;
}

void Entity::update(float dt, Sample_Mixer *mixer, Tile_Grid *grid)
{
    if (state == Entity_State::Alive && ground(grid)) {
        particles.current_color = get_particle_color_for_tile(grid, feet());
        if (alive_state == Alive_State::Walking) {
            particles.state = Particles::EMITTING;
        } else {
            particles.state = Particles::DISABLED;
        }
    } else {
        particles.state = Particles::DISABLED;
    }

    if (state == Entity_State::Alive && ground(grid)) {
        this->count_jumps = 0;
    }

    particles.source = feet();
    particles.update(dt, grid);

    switch (state) {
    case Entity_State::Alive: {
        switch (jump_state) {
        case Jump_State::No_Jump:
            break;

        case Jump_State::Prepare:
            prepare_for_jump_animat.update(dt);
            if (prepare_for_jump_animat.finished()) {
                jump_animat.reset();
                jump_state = Jump_State::Jump;
                has_jumped = true;
                vel.y = ENTITY_GRAVITY * -0.6f;
                mixer->play_sample(jump_samples[rand() % 2]);
                if (ground(grid)) {
                    for (int i = 0; i < ENTITY_JUMP_PARTICLE_BURST; ++i) {
                        particles.push(rand_float_range(PARTICLE_JUMP_VEL_LOW, PARTICLE_JUMP_VEL_HIGH));
                    }
                }
            }
            break;

        case Jump_State::Jump:
            jump_animat.update(dt);
            if (jump_animat.finished()) {
                jump_state = Jump_State::No_Jump;
            }
            break;
        }

        switch (alive_state) {
        case Alive_State::Idle: {
            // TODO(#270): entities should own their own copies of animats
            //
            // Right now we are mutating instance of animats that are inside of Assets.
            // And those could be used by other entities. And they will fight with each other.
            idle.update(dt);
        } break;

        case Alive_State::Walking: {
            const float ENTITY_ACCEL = speed * ENTITY_ACCEL_FACTOR;
            switch (walking_direction) {
            case Left: {
                vel.x = -fmax(vel.x + ENTITY_ACCEL * dt, speed);
            } break;

            case Right: {
                vel.x = fminf(vel.x + ENTITY_ACCEL * dt, speed);
            } break;
            }

            walking.update(dt);
        } break;

        case Alive_State::Stomping: {
            vel.y += ENTITY_STOMP_ACCEL * dt;
            if (ground(grid)) {
                // assert(0 && "TODO: impact");
                alive_state = Alive_State::Idle;
            }
        } break;
        }

        flash_alpha = fmax(0.0f, flash_alpha - ENTITY_FLASH_ALPHA_DECAY * dt);

        if (!noclip) {
            vel.y += ENTITY_GRAVITY * dt;
        }

        const float ENTITY_DECEL = speed * ENTITY_DECEL_FACTOR;
        const float ENTITY_STOP_THRESHOLD = speed / 8.0f;
        if (fabs(vel.x) > ENTITY_STOP_THRESHOLD) {
            vel.x -= sgn(vel.x) * ENTITY_DECEL * dt;
        } else {
            vel.x = 0.0f;
        }

        pos += vel * dt;
        cooldown_weapon -= dt;
    } break;

    case Entity_State::Poof: {
        poof_animat.update(dt);
        if (poof_animat.finished()) {
            state = Entity_State::Ded;
        }
    } break;

    case Entity_State::Ded: {} break;
    }
}

void Entity::point_gun_at(Vec2f target)
{
    gun_dir = target - pos;
}

void Entity::jump()
{
    switch (state) {
    case Entity_State::Alive: {
        switch (alive_state) {
        case Alive_State::Idle:
        case Alive_State::Walking: {
            if (this->count_jumps < this->max_allowed_jumps) {
                this->count_jumps++;

                if (jump_state == Jump_State::No_Jump) {
                    prepare_for_jump_animat.reset();
                    jump_state = Jump_State::Prepare;
                }
            }
        } break;

        case Alive_State::Stomping: {} break;
        }
    } break;

    case Entity_State::Poof:
    case Entity_State::Ded: {} break;
    }

}

Entity player_entity(Vec2f pos)
{
    Entity entity = {};

    entity.push_weapon(water_gun());
    entity.push_weapon(fire_gun());
    entity.push_weapon(ice_block_placer(20));
    entity.push_weapon(dirt_block_placer(20));
    entity.push_weapon(ice_block_placer(20));

    entity.texbox_local.w = PLAYER_TEXBOX_W;
    entity.texbox_local.h = PLAYER_TEXBOX_H;
    entity.hitbox_local.w = PLAYER_HITBOX_W;
    entity.hitbox_local.h = PLAYER_HITBOX_H;
    entity.texbox_local.x = entity.texbox_local.w * -0.5f;
    entity.texbox_local.y = entity.texbox_local.h * -0.5f;
    entity.hitbox_local.x = entity.hitbox_local.w * -0.5f;
    entity.hitbox_local.y = entity.hitbox_local.h * -0.5f;
    entity.speed = 800.0f;

    entity.idle            = frames_animat(PLAYER_ANIMAT_INDEX);
    entity.walking         = frames_animat(PLAYER_ANIMAT_INDEX);
    entity.jump_samples[0] = JUMP1_SOUND_INDEX;
    entity.jump_samples[1] = JUMP2_SOUND_INDEX;

    entity.lives = ENTITY_INITIAL_LIVES;
    entity.state = Entity_State::Alive;
    entity.alive_state = Alive_State::Idle;
    entity.pos = pos;
    entity.gun_dir = vec2(1.0f, 0.0f);

    /*
     * TODO(#265): We should defined the number of max allowed jump in configuration file
     */
    entity.count_jumps = 0;
    entity.max_allowed_jumps = PLAYER_ENTITY_MAX_JUMPS;

    entity.prepare_for_jump_animat.begin = 0.0f;
    entity.prepare_for_jump_animat.end = 0.2f;
    entity.prepare_for_jump_animat.duration = 0.05f;

    entity.jump_animat.rubber_animats[0].begin = 0.2f;
    entity.jump_animat.rubber_animats[0].end = -0.2f;
    entity.jump_animat.rubber_animats[0].duration = 0.1f;

    entity.jump_animat.rubber_animats[1].begin = -0.2f;
    entity.jump_animat.rubber_animats[1].end = 0.0f;
    entity.jump_animat.rubber_animats[1].duration = 0.05f;

    entity.poof_animat.begin = 0.0f;
    entity.poof_animat.end = 1.0f;
    entity.poof_animat.duration = 0.1f;

    return entity;
}

Entity ice_golem_entity(Vec2f pos)
{
    Entity entity = {};

    entity.push_weapon(ice_gun());
    entity.push_item(make_ice_block_item());

    entity.texbox_local.w = ENEMY_TEXBOX_W + 32.0f;
    entity.texbox_local.h = ENEMY_TEXBOX_H + 32.0f;
    entity.hitbox_local.w = ENEMY_HITBOX_W + 32.0f;
    entity.hitbox_local.h = ENEMY_HITBOX_H + 32.0f;
    entity.texbox_local.x = entity.texbox_local.w * -0.5f;
    entity.texbox_local.y = entity.texbox_local.h * -0.5f;
    entity.hitbox_local.x = entity.hitbox_local.w * -0.5f;
    entity.hitbox_local.y = entity.hitbox_local.h * -0.5f;
    entity.speed = 800.0f;

    entity.idle    = frames_animat(ICE_GOLEM_IDLE_ANIMAT_INDEX);
    entity.walking = frames_animat(ICE_GOLEM_WALKING_ANIMAT_INDEX);
    entity.jump_samples[0] = JUMP1_SOUND_INDEX;
    entity.jump_samples[1] = JUMP2_SOUND_INDEX;

    entity.lives = ENTITY_INITIAL_LIVES;
    entity.state = Entity_State::Alive;
    entity.alive_state = Alive_State::Idle;
    entity.pos = pos;
    entity.gun_dir = vec2(1.0f, 0.0f);

    /*
     * Ice Golems could have more consecutives jumps then other golems.
     * The amount of jumps could be increase by level in both players and enimies
     */
    entity.count_jumps = 0;
    entity.max_allowed_jumps = ICE_GOLEM_ENTITY_MAX_JUMPS;

    entity.prepare_for_jump_animat.begin = 0.0f;
    entity.prepare_for_jump_animat.end = 0.2f;
    entity.prepare_for_jump_animat.duration = 0.2f;

    entity.jump_animat.rubber_animats[0].begin = 0.2f;
    entity.jump_animat.rubber_animats[0].end = -0.2f;
    entity.jump_animat.rubber_animats[0].duration = 0.1f;

    entity.jump_animat.rubber_animats[1].begin = -0.2f;
    entity.jump_animat.rubber_animats[1].end = 0.0f;
    entity.jump_animat.rubber_animats[1].duration = 0.2f;

    entity.poof_animat.begin = 0.0f;
    entity.poof_animat.end = 1.0f;
    entity.poof_animat.duration = 0.1f;

    return entity;
}

Entity golem_entity(Vec2f pos)
{
    Entity entity = {};

    entity.push_weapon(rock_gun());
    entity.push_item(make_dirt_block_item());

    entity.texbox_local.w = ENEMY_TEXBOX_W + 32.0f;
    entity.texbox_local.h = ENEMY_TEXBOX_H + 32.0f;
    entity.hitbox_local.w = ENEMY_HITBOX_W + 32.0f;
    entity.hitbox_local.h = ENEMY_HITBOX_H + 32.0f;
    entity.texbox_local.x = entity.texbox_local.w * -0.5f;
    entity.texbox_local.y = entity.texbox_local.h * -0.5f;
    entity.hitbox_local.x = entity.hitbox_local.w * -0.5f;
    entity.hitbox_local.y = entity.hitbox_local.h * -0.5f;
    entity.speed = 400.0f;

    entity.idle    = frames_animat(DIRT_GOLEM_ANIMAT_INDEX);
    entity.walking = frames_animat(DIRT_GOLEM_ANIMAT_INDEX);
    entity.jump_samples[0] = JUMP1_SOUND_INDEX;
    entity.jump_samples[1] = JUMP2_SOUND_INDEX;

    entity.lives = ENTITY_INITIAL_LIVES;
    entity.state = Entity_State::Alive;
    entity.alive_state = Alive_State::Idle;
    entity.pos = pos;
    entity.gun_dir = vec2(1.0f, 0.0f);

    /*
     * "Traditional" Golems could start just able to jump once.
     */
    entity.count_jumps = 0;
    entity.max_allowed_jumps = GOLEM_ENTITY_MAX_JUMPS;

    entity.prepare_for_jump_animat.begin = 0.0f;
    entity.prepare_for_jump_animat.end = 0.2f;
    entity.prepare_for_jump_animat.duration = 0.2f;

    entity.jump_animat.rubber_animats[0].begin = 0.2f;
    entity.jump_animat.rubber_animats[0].end = -0.2f;
    entity.jump_animat.rubber_animats[0].duration = 0.1f;

    entity.jump_animat.rubber_animats[1].begin = -0.2f;
    entity.jump_animat.rubber_animats[1].end = 0.0f;
    entity.jump_animat.rubber_animats[1].duration = 0.2f;

    entity.poof_animat.begin = 0.0f;
    entity.poof_animat.end = 1.0f;
    entity.poof_animat.duration = 0.1f;

    return entity;
}

Entity enemy_entity(Vec2f pos)
{
    Entity entity = {};

    entity.push_weapon(water_gun());

    entity.texbox_local.w = ENEMY_TEXBOX_W;
    entity.texbox_local.h = ENEMY_TEXBOX_H;
    entity.hitbox_local.w = ENEMY_HITBOX_W;
    entity.hitbox_local.h = ENEMY_HITBOX_H;
    entity.texbox_local.x = entity.texbox_local.w * -0.5f;
    entity.texbox_local.y = entity.texbox_local.h * -0.5f;
    entity.hitbox_local.x = entity.hitbox_local.w * -0.5f;
    entity.hitbox_local.y = entity.hitbox_local.h * -0.5f;
    entity.speed = 800.0f;

    entity.idle            = frames_animat(ENEMY_IDLE_ANIMAT_INDEX);
    entity.walking         = frames_animat(ENEMY_WALKING_ANIMAT_INDEX);
    entity.jump_samples[0] = JUMP1_SOUND_INDEX;
    entity.jump_samples[1] = JUMP2_SOUND_INDEX;

    entity.lives = ENTITY_INITIAL_LIVES;
    entity.state = Entity_State::Alive;
    entity.alive_state = Alive_State::Idle;
    entity.pos = pos;
    entity.gun_dir = vec2(1.0f, 0.0f);

    /*
     * For default the enemy should have the same amount of jumps
     * that a player
     */
    entity.count_jumps = 0;
    entity.max_allowed_jumps = ENEMY_ENTITY_MAX_JUMPS;

    entity.prepare_for_jump_animat.begin = 0.0f;
    entity.prepare_for_jump_animat.end = 0.2f;
    entity.prepare_for_jump_animat.duration = 0.2f;

    entity.jump_animat.rubber_animats[0].begin = 0.2f;
    entity.jump_animat.rubber_animats[0].end = -0.2f;
    entity.jump_animat.rubber_animats[0].duration = 0.1f;

    entity.jump_animat.rubber_animats[1].begin = -0.2f;
    entity.jump_animat.rubber_animats[1].end = 0.0f;
    entity.jump_animat.rubber_animats[1].duration = 0.2f;

    entity.poof_animat.begin = 0.0f;
    entity.poof_animat.end = 1.0f;
    entity.poof_animat.duration = 0.1f;

    return entity;
}

void Entity::flash(RGBA color)
{
    flash_alpha = 1.0f;
    flash_color = color;
}

void Entity::move(Direction direction)
{
    if (alive_state != Alive_State::Stomping) {
        alive_state = Alive_State::Walking;
        walking_direction = direction;
    }
}

void Entity::stop()
{
    if (alive_state != Alive_State::Stomping) {
        alive_state = Alive_State::Idle;
    }
}

Vec2f Entity::feet()
{
    auto hitbox = hitbox_local;
    hitbox.x += pos.x;
    hitbox.y += pos.y;
    return vec2(hitbox.x, hitbox.y) + vec2(0.5f, 1.0f) * vec2(hitbox.w, hitbox.h);
}

bool Entity::ground(Tile_Grid *grid)
{
    return !grid->is_tile_empty_abs(feet() + vec2(0.0f, TILE_SIZE * 0.5f));
}

Weapon *Entity::get_current_weapon()
{
    if (weapon_current < weapon_slots_count) {
        return &weapon_slots[weapon_current];
    } else {
        return NULL;
    }
}

void Entity::push_weapon(Weapon weapon)
{
    assert(weapon_slots_count < WEAPON_SLOTS_CAPACITY);
    weapon_slots[weapon_slots_count++] = weapon;
}

void Entity::stomp(Tile_Grid *grid)
{
    if (state == Entity_State::Alive &&
        alive_state != Alive_State::Stomping && // can't stomp is you are alreading stomping
        jump_state == Jump_State::No_Jump &&
        !ground(grid))
    {
        alive_state = Alive_State::Stomping;
    }
}
