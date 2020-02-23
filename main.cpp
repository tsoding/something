#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <SDL.h>
#include <SDL_ttf.h>

#include <png.h>

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

constexpr int LEVEL_WIDTH = 6;
constexpr int LEVEL_HEIGHT = 5;

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

void render_level(SDL_Renderer *renderer, Sprite wall_texture)
{
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            switch (level[y][x]) {
            case Tile::Empty: {
            } break;

            case Tile::Wall: {
                sec(SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255));
                render_sprite(renderer, wall_texture,
                                    {x * TILE_SIZE, y * TILE_SIZE,
                                     TILE_SIZE, TILE_SIZE});
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
    printf("Width: %d, Height: %d\n", image.width, image.height);
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
    SDL_Rect hitbox;
    int dx, dy;
};

static inline
bool is_not_oob(int x, int y)
{
    return 0 <= x && x < LEVEL_WIDTH && 0 <= y && y < LEVEL_HEIGHT;
}

bool is_tile_empty(int x, int y)
{
    return !is_not_oob(x, y) || level[y][x] == Tile::Empty;
}

static inline
int sqr_dist(int x0, int y0, int x1, int y1)
{
    int dx = x0 - x1;
    int dy = y0 - y1;
    return dx * dx + dy * dy;
}

void resolve_point_collision(int *x, int *y)
{
    assert(x);
    assert(y);

    const int tile_x = *x / TILE_SIZE;
    const int tile_y = *y / TILE_SIZE;

    if (is_tile_empty(tile_x, tile_y)) {
        return;
    }

    const int x0 = tile_x * TILE_SIZE;
    const int x1 = (tile_x + 1) * TILE_SIZE;
    const int y0 = tile_y * TILE_SIZE;
    const int y1 = (tile_y + 1) * TILE_SIZE;

    struct Side { int d, x, y, dx, dy, dd; };

    Side sides[] = {
        {sqr_dist(x0, 0, *x, 0),   x0, *y, -1,  0, TILE_SIZE_SQR},     // left
        {sqr_dist(x1, 0, *x, 0),   x1, *y,  1,  0, TILE_SIZE_SQR},     // right
        {sqr_dist(0, y0, 0, *y),   *x, y0,  0, -1, TILE_SIZE_SQR},     // top
        {sqr_dist(0, y1, 0, *y),   *x, y1,  0,  1, TILE_SIZE_SQR},     // bottom
        {sqr_dist(x0, y0, *x, *y), x0, y0, -1, -1, TILE_SIZE_SQR * 2}, // top-left
        {sqr_dist(x1, y0, *x, *y), x1, y0,  1, -1, TILE_SIZE_SQR * 2}, // top-right
        {sqr_dist(x0, y1, *x, *y), x0, y1, -1,  1, TILE_SIZE_SQR * 2}, // bottom-left
        {sqr_dist(x1, y1, *x, *y), x1, y1,  1,  1, TILE_SIZE_SQR * 2}  // bottom-right
    };
    constexpr int SIDES_COUNT = sizeof(sides) / sizeof(sides[0]);

    int closest = -1;
    for (int current = 0; current < SIDES_COUNT; ++current) {
        for (int i = 1;
             !is_tile_empty(tile_x + sides[current].dx * i,
                            tile_y + sides[current].dy * i);
             ++i)
        {
            sides[current].d += sides[current].dd;
        }

        if (closest < 0 || sides[closest].d >= sides[current].d) {
            closest = current;
        }
    }

    *x = sides[closest].x;
    *y = sides[closest].y;
}

void resolve_player_collision(Player *player)
{
    assert(player);

    int x0 = player->hitbox.x;
    int y0 = player->hitbox.y;
    int x1 = player->hitbox.x + player->hitbox.w;
    int y1 = player->hitbox.y + player->hitbox.h;

    int mesh[][2] = {
        {x0, y0},
        {x1, y0},
        {x0, y1},
        {x1, y1},
    };
    constexpr int MESH_COUNT = sizeof(mesh) / sizeof(mesh[0]);
    constexpr int X = 0;
    constexpr int Y = 1;

    for (int i = 0; i < MESH_COUNT; ++i) {
        int tx = mesh[i][X];
        int ty = mesh[i][Y];
        resolve_point_collision(&tx, &ty);
        int dx = tx - mesh[i][X];
        int dy = ty - mesh[i][Y];

        constexpr int IMPACT_THRESHOLD = 5;
        if (std::abs(dy) >= IMPACT_THRESHOLD) player->dy = 0;
        if (std::abs(dx) >= IMPACT_THRESHOLD) player->dx = 0;

        for (int j = 0; j < MESH_COUNT; ++j) {
            mesh[j][X] += dx;
            mesh[j][Y] += dy;
        }
    }

    static_assert(MESH_COUNT >= 1);

    player->hitbox.x = mesh[0][X];
    player->hitbox.y = mesh[0][Y];
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

void render_texture(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y)
{
    int w, h;
    sec(SDL_QueryTexture(texture, NULL, NULL, &w, &h));
    SDL_Rect srcrect = {0, 0, w, h};
    SDL_Rect dstrect = {x, y, w, h};
    sec(SDL_RenderCopy(renderer, texture, &srcrect, &dstrect));
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

    Sprite wall_texture = {
        {120, 128, 16, 16},
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

    Animat walking = {};
    walking.frames = walking_frames;
    walking.frame_count = 4;
    walking.frame_duration = 100;

    Animat idle = {};
    idle.frames = walking_frames + 2;
    idle.frame_count = 1;
    idle.frame_duration = 200;

    Player player = {};
    player.dy = 0;
    player.hitbox = {0, 0, 64, 64};

    stec(TTF_Init());
    TTF_Font *font = stec(TTF_OpenFont("assets/UbuntuMono-R.ttf", 69));
    SDL_Texture *hello_world_texture =
        render_text_as_texture(renderer, font, "Welcome to my Dungeon", {255, 0, 0, 255});

    Animat *current = &idle;
    int ddy = 1;
    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
    SDL_RendererFlip player_dir = SDL_FLIP_NONE;
    bool quit = false;
    bool debug = false;
    constexpr int CURSOR_SIZE = 10;
    SDL_Rect cursor = {};
    SDL_Rect tile_rect = {};

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
                    player.dy = -20;
                } break;

                case SDLK_q: {
                    debug = !debug;
                } break;

                case SDLK_r: {
                    player.hitbox.x = 0;
                    player.hitbox.y = 0;
                    player.dy = 0;
                } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                auto x = event.motion.x;
                auto y = event.motion.y;

                resolve_point_collision(&x, &y);

                cursor = {
                    x - CURSOR_SIZE, y - CURSOR_SIZE,
                    CURSOR_SIZE * 2, CURSOR_SIZE * 2
                };

                tile_rect = {
                    event.motion.x / TILE_SIZE * TILE_SIZE,
                    event.motion.y / TILE_SIZE * TILE_SIZE,
                    TILE_SIZE, TILE_SIZE
                };
            } break;
            }
        }

        if (keyboard[SDL_SCANCODE_D]) {
            player.dx = PLAYER_SPEED;
            current = &walking;
            player_dir = SDL_FLIP_HORIZONTAL;
        } else if (keyboard[SDL_SCANCODE_A]) {
            player.dx = -PLAYER_SPEED;
            current = &walking;
            player_dir = SDL_FLIP_NONE;
        } else {
            player.dx = 0;
            current = &idle;
        }

        player.dy += ddy;

        player.hitbox.x += player.dx;
        player.hitbox.y += player.dy;

        resolve_player_collision(&player);

        sec(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
        sec(SDL_RenderClear(renderer));

        render_level(renderer, wall_texture);
        render_animat(renderer, *current, player.hitbox, player_dir);

        if (debug) {
            sec(SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255));

            sec(SDL_RenderDrawRect(renderer, &player.hitbox));
            sec(SDL_RenderFillRect(renderer, &cursor));
            sec(SDL_RenderDrawRect(renderer, &tile_rect));
        }

        render_texture(renderer, hello_world_texture, 200, 200);

        SDL_RenderPresent(renderer);

        const Uint32 dt = SDL_GetTicks() - begin;

        update_animat(current, dt);
    }
    SDL_Quit();

    return 0;
}
