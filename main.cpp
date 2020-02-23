#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <SDL.h>
#include <SDL_ttf.h>

#include <png.h>

#include "vec.hpp"

template <typename T>
T *stec(T *ptr)
{
    if (ptr == nullptr) {
        fprintf(stderr, "SDL_ttf pooped itself: %s\n", TTF_GetError());
        abort();
    }

    return ptr;
}

void stec(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL_ttf pooped itself: %s\n", TTF_GetError());
        abort();
    }
}

void sec(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }
}

template <typename T>
T *sec(T *ptr)
{
    if (ptr == nullptr) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }

    return ptr;
}

constexpr int TILE_SIZE = 128;
constexpr int TILE_SIZE_SQR = TILE_SIZE * TILE_SIZE;

enum class Tile
{
    Empty = 0,
    Wall
};

constexpr int LEVEL_WIDTH = 10;
constexpr int LEVEL_HEIGHT = 10;
constexpr SDL_Rect level_boundary = {
    0, 0, LEVEL_WIDTH * TILE_SIZE, LEVEL_HEIGHT * TILE_SIZE
};

Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Wall,  Tile::Empty, Tile::Empty, Tile::Wall,  Tile::Empty},
    {Tile::Empty, Tile::Wall,  Tile::Empty, Tile::Wall,  Tile::Empty, Tile::Wall},
    {Tile::Wall,  Tile::Wall,  Tile::Empty, Tile::Empty, Tile::Wall , Tile::Empty},
};

struct Sprite
{
    SDL_Rect srcrect;
    SDL_Texture *texture;
};

void render_sprite(SDL_Renderer *renderer,
                   Sprite texture,
                   SDL_Rect destrect,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    sec(SDL_RenderCopyEx(
            renderer,
            texture.texture,
            &texture.srcrect,
            &destrect,
            0.0,
            nullptr,
            flip));
}

static inline
bool is_not_oob(Vec2i p)
{
    return 0 <= p.x && p.x < LEVEL_WIDTH && 0 <= p.y && p.y < LEVEL_HEIGHT;
}

bool is_tile_empty(Vec2i p)
{
    return !is_not_oob(p) || level[p.y][p.x] == Tile::Empty;
}

void render_level(SDL_Renderer *renderer,
                  Sprite top_ground_texture,
                  Sprite bottom_ground_texture)
{
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            switch (level[y][x]) {
            case Tile::Empty: {
            } break;

            case Tile::Wall: {
                if (is_tile_empty(vec2(x, y - 1))) {
                    render_sprite(renderer, top_ground_texture,
                                  {x * TILE_SIZE, y * TILE_SIZE,
                                   TILE_SIZE, TILE_SIZE});
                } else {
                    render_sprite(renderer, bottom_ground_texture,
                                  {x * TILE_SIZE, y * TILE_SIZE,
                                   TILE_SIZE, TILE_SIZE});
                }
            } break;
            }
        }
    }
}

SDL_Texture *load_texture_from_png_file(SDL_Renderer *renderer, const char *image_filename)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    // TODO(#6): implement libpng error checker similar to the SDL one
    // TODO(#7): try stb_image.h instead of libpng
    //   https://github.com/nothings/stb/blob/master/stb_image.h
    if (!png_image_begin_read_from_file(&image, image_filename)) {
        fprintf(stderr, "Could not read file `%s`: %s\n",
                image_filename, image.message);
        abort();
    }
    image.format = PNG_FORMAT_RGBA;
    uint32_t *image_pixels = new uint32_t[image.width * image.height];

    if (!png_image_finish_read(&image, nullptr, image_pixels, 0, nullptr)) {
        fprintf(stderr, "libpng pooped itself: %s\n", image.message);
        abort();
    }

    SDL_Surface* image_surface =
        sec(SDL_CreateRGBSurfaceFrom(image_pixels,
                                     image.width,
                                     image.height,
                                     32,
                                     image.width * 4,
                                     0x000000FF,
                                     0x0000FF00,
                                     0x00FF0000,
                                     0xFF000000));

    SDL_Texture *image_texture =
        sec(SDL_CreateTextureFromSurface(renderer,
                                         image_surface));
    SDL_FreeSurface(image_surface);

    return image_texture;
}

struct Animat
{
    Sprite  *frames;
    size_t   frame_count;
    size_t   frame_current;
    uint32_t frame_duration;
    uint32_t frame_cooldown;
};

static inline
void render_animat(SDL_Renderer *renderer,
                   Animat animat,
                   SDL_Rect dstrect,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    render_sprite(
        renderer,
        animat.frames[animat.frame_current % animat.frame_count],
        dstrect,
        flip);
}

void update_animat(Animat *animat, uint32_t dt)
{
    if (dt < animat->frame_cooldown) {
        animat->frame_cooldown -= dt;
    } else {
        animat->frame_current = (animat->frame_current + 1) % animat->frame_count;
        animat->frame_cooldown = animat->frame_duration;
    }
}

struct Player
{
    SDL_Rect texbox;
    SDL_Rect hitbox;
    Vec2i pos;
    Vec2i vel;

    Animat idle;
    Animat walking;
    Animat *current;
    SDL_RendererFlip dir;
};

static inline
int sqr_dist(Vec2i p0, Vec2i p1)
{
    auto d = p0 - p1;
    return d.x * d.x + d.y * d.y;
}

void resolve_point_collision(Vec2i *p)
{
    assert(p);

    const auto tile = *p / TILE_SIZE;

    if (is_tile_empty(tile)) {
        return;
    }

    const auto p0 = tile * TILE_SIZE;
    const auto p1 = (tile + 1) * TILE_SIZE;

    struct Side {
        int d;
        Vec2i np;
        Vec2i nd;
        int dd;
    };

    Side sides[] = {
        {sqr_dist({p0.x, 0},    {p->x, 0}),    {p0.x, p->y}, {-1,  0}, TILE_SIZE_SQR},     // left
        {sqr_dist({p1.x, 0},    {p->x, 0}),    {p1.x, p->y}, { 1,  0}, TILE_SIZE_SQR},     // right
        {sqr_dist({0, p0.y},    {0, p->y}),    {p->x, p0.y}, { 0, -1}, TILE_SIZE_SQR},     // top
        {sqr_dist({0, p1.y},    {0, p->y}),    {p->x, p1.y}, { 0,  1}, TILE_SIZE_SQR},     // bottom
        {sqr_dist({p0.x, p0.y}, {p->x, p->y}), {p0.x, p0.y}, {-1, -1}, TILE_SIZE_SQR * 2}, // top-left
        {sqr_dist({p1.x, p0.y}, {p->x, p->y}), {p1.x, p0.y}, { 1, -1}, TILE_SIZE_SQR * 2}, // top-right
        {sqr_dist({p0.x, p1.y}, {p->x, p->y}), {p0.x, p1.y}, {-1,  1}, TILE_SIZE_SQR * 2}, // bottom-left
        {sqr_dist({p1.x, p1.y}, {p->x, p->y}), {p1.x, p1.y}, { 1,  1}, TILE_SIZE_SQR * 2}  // bottom-right
    };
    constexpr int SIDES_COUNT = sizeof(sides) / sizeof(sides[0]);

    int closest = -1;
    for (int current = 0; current < SIDES_COUNT; ++current) {
        for (int i = 1;
             !is_tile_empty(tile + (sides[current].nd * i)) ;
             ++i)
        {
            sides[current].d += sides[current].dd;
        }

        if (closest < 0 || sides[closest].d >= sides[current].d) {
            closest = current;
        }
    }

    *p = sides[closest].np;
}

void resolve_player_collision(Player *player)
{
    assert(player);

    Vec2i p0 = vec2(player->hitbox.x, player->hitbox.y) + player->pos;
    Vec2i p1 = p0 + vec2(player->hitbox.w, player->hitbox.h);

    Vec2i mesh[] = {
        p0,
        {p1.x, p0.y},
        {p0.x, p1.y},
        p1,
    };
    constexpr int MESH_COUNT = sizeof(mesh) / sizeof(mesh[0]);

    for (int i = 0; i < MESH_COUNT; ++i) {
        Vec2i t = mesh[i];
        resolve_point_collision(&t);
        Vec2i d = t - mesh[i];

        constexpr int IMPACT_THRESHOLD = 5;
        if (std::abs(d.y) >= IMPACT_THRESHOLD) player->vel.y = 0;
        if (std::abs(d.x) >= IMPACT_THRESHOLD) player->vel.x = 0;

        for (int j = 0; j < MESH_COUNT; ++j) {
            mesh[j] += d;
        }

        player->pos += d;
    }
}

SDL_Rect player_dstrect(const Player player)
{
    SDL_Rect dstrect = {
        player.texbox.x + player.pos.x, player.texbox.y + player.pos.y,
        player.texbox.w, player.texbox.h
    };
    return dstrect;
}

SDL_Rect player_hitbox(const Player player)
{
    SDL_Rect hitbox = {
        player.hitbox.x + player.pos.x, player.hitbox.y + player.pos.y,
        player.hitbox.w, player.hitbox.h
    };
    return hitbox;
}

void render_player(SDL_Renderer *renderer, const Player player)
{
    auto dstrect = player_dstrect(player);
    render_animat(renderer, *player.current, dstrect, player.dir);
}

void update_player(Player *player, uint32_t dt)
{
    assert(player);
    update_animat(player->current, dt);
}

SDL_Texture *render_text_as_texture(SDL_Renderer *renderer,
                                    TTF_Font *font,
                                    const char *text,
                                    SDL_Color color)
{
    SDL_Surface *surface = stec(TTF_RenderText_Blended(font, text, color));
    SDL_Texture *texture = stec(SDL_CreateTextureFromSurface(renderer, surface));
    SDL_FreeSurface(surface);
    return texture;
}

void render_texture(SDL_Renderer *renderer, SDL_Texture *texture, Vec2i p)
{
    int w, h;
    sec(SDL_QueryTexture(texture, NULL, NULL, &w, &h));
    SDL_Rect srcrect = {0, 0, w, h};
    SDL_Rect dstrect = {p.x, p.y, w, h};
    sec(SDL_RenderCopy(renderer, texture, &srcrect, &dstrect));
}

void displayf(SDL_Renderer *renderer, TTF_Font *font,
              SDL_Color color, Vec2i p,
              const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char text[256];
    vsnprintf(text, sizeof(text), format, args);

    SDL_Texture *texture =
        render_text_as_texture(renderer, font, text, color);
    render_texture(renderer, texture, p);
    SDL_DestroyTexture(texture);

    va_end(args);
}

int main(void)
{
    sec(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window =
        sec(SDL_CreateWindow(
                "Something",
                0, 0, 800, 600,
                SDL_WINDOW_RESIZABLE));

    SDL_Renderer *renderer =
        sec(SDL_CreateRenderer(
                window, -1,
                SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

    // TODO(#8): replace fantasy_tiles.png with our own assets
    SDL_Texture *tileset_texture = load_texture_from_png_file(
        renderer,
        "assets/fantasy_tiles.png");

    Sprite ground_grass_texture = {
        {120, 128, 16, 16},
        tileset_texture
    };

    Sprite ground_texture = {
        {120, 128 + 16, 16, 16},
        tileset_texture
    };

    // TODO(#9): baking assets into executable
    SDL_Texture *walking_texture = load_texture_from_png_file(
        renderer,
        "assets/walking-12px-zoom.png");

    constexpr int walking_frame_count = 4;
    constexpr int walking_frame_size = 48;
    Sprite walking_frames[walking_frame_count];

    for (int i = 0; i < walking_frame_count; ++i) {
        walking_frames[i].srcrect = {
            i * walking_frame_size,
            0,
            walking_frame_size,
            walking_frame_size
        };
        walking_frames[i].texture = walking_texture;
    }

    constexpr int PLAYER_TEXBOX_SIZE = 64;
    constexpr int PLAYER_HITBOX_SIZE = PLAYER_TEXBOX_SIZE - 20;
    Player player = {};
    player.texbox = {
        - (PLAYER_TEXBOX_SIZE / 2), - (PLAYER_TEXBOX_SIZE / 2),
        PLAYER_TEXBOX_SIZE, PLAYER_TEXBOX_SIZE
    };
    player.hitbox = {
        - (PLAYER_HITBOX_SIZE / 2), - (PLAYER_HITBOX_SIZE / 2),
        PLAYER_HITBOX_SIZE, PLAYER_HITBOX_SIZE
    };
    player.walking.frames = walking_frames;
    player.walking.frame_count = 4;
    player.walking.frame_duration = 100;
    player.idle.frames = walking_frames + 2;
    player.idle.frame_count = 1;
    player.idle.frame_duration = 200;
    player.current = &player.idle;
    player.dir = SDL_FLIP_NONE;

    stec(TTF_Init());
    constexpr int DEBUG_FONT_SIZE = 32;
    TTF_Font *debug_font = stec(TTF_OpenFont("assets/UbuntuMono-R.ttf", DEBUG_FONT_SIZE));

    int ddy = 1;
    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

    bool quit = false;
    bool debug = false;
    constexpr int COLLISION_PROBE_SIZE = 10;
    SDL_Rect collision_probe = {};
    Vec2i mouse_position = {};
    SDL_Rect tile_rect = {};

    Uint32 fps = 0;
    while (!quit) {
        const Uint32 begin = SDL_GetTicks();

        constexpr int PLAYER_SPEED = 4;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                quit = true;
            } break;

            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    player.vel.y = -20;
                } break;

                case SDLK_q: {
                    debug = !debug;
                } break;

                case SDLK_r: {
                    player.pos = vec2(0, 0);
                    player.vel.y = 0;
                } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                Vec2i p = {event.motion.x, event.motion.y};
                resolve_point_collision(&p);

                collision_probe = {
                    p.x - COLLISION_PROBE_SIZE, p.y - COLLISION_PROBE_SIZE,
                    COLLISION_PROBE_SIZE * 2, COLLISION_PROBE_SIZE * 2
                };

                tile_rect = {
                    event.motion.x / TILE_SIZE * TILE_SIZE,
                    event.motion.y / TILE_SIZE * TILE_SIZE,
                    TILE_SIZE, TILE_SIZE
                };

                mouse_position = {event.motion.x, event.motion.y};
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                if (debug) {
                    Vec2i tile = vec2(event.button.x, event.button.y) / TILE_SIZE;
                    if (is_not_oob(tile)) {
                        if (level[tile.y][tile.x] == Tile::Empty) {
                            level[tile.y][tile.x] = Tile::Wall;
                        } else {
                            level[tile.y][tile.x] = Tile::Empty;
                        }
                    }
                }
            } break;
            }
        }

        if (keyboard[SDL_SCANCODE_D]) {
            player.vel.x = PLAYER_SPEED;
            player.current = &player.walking;
            player.dir = SDL_FLIP_NONE;
        } else if (keyboard[SDL_SCANCODE_A]) {
            player.vel.x = -PLAYER_SPEED;
            player.current = &player.walking;
            player.dir = SDL_FLIP_HORIZONTAL;
        } else {
            player.vel.x = 0;
            player.current = &player.idle;
        }

        player.vel.y += ddy;

        player.pos += player.vel;

        resolve_player_collision(&player);

        sec(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
        sec(SDL_RenderClear(renderer));

        render_level(renderer, ground_grass_texture, ground_texture);
        render_player(renderer, player);

        if (debug) {
            sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));

            auto dstrect = player_dstrect(player);
            sec(SDL_RenderDrawRect(renderer, &dstrect));
            sec(SDL_RenderFillRect(renderer, &collision_probe));
            sec(SDL_RenderDrawRect(renderer, &tile_rect));
            sec(SDL_RenderDrawRect(renderer, &level_boundary));

            const Uint32 t = SDL_GetTicks() - begin;
            const Uint32 fps_snapshot = t ? 1000 / t : 0;
            fps = (fps + fps_snapshot) / 2;

            constexpr int PADDING = 10;
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, PADDING),
                     "FPS: %d", fps);
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, 50 + PADDING),
                     "Mouse Position: (%d, %d)",
                     mouse_position.x, mouse_position.y);
            displayf(renderer, debug_font,
                     {255, 0, 0, 255}, vec2(PADDING, 2 * 50 + PADDING),
                     "Collision Probe: (%d, %d)",
                     collision_probe.x, collision_probe.y);

            sec(SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255));
            auto hitbox = player_hitbox(player);
            sec(SDL_RenderDrawRect(renderer, &hitbox));
        }


        SDL_RenderPresent(renderer);

        const Uint32 dt = SDL_GetTicks() - begin;

        update_player(&player, dt);
    }
    SDL_Quit();

    return 0;
}
