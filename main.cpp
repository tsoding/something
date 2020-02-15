#include <cstdio>
#include <cstdlib>
#include <SDL.h>

#include <png.h>

// TODO: rename sdl -> sec (SDL Error Check)
int sdl(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }

    return code;
}

template <typename T>
T *sdl(T *ptr)
{
    if (ptr == nullptr) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }

    return ptr;
}

constexpr int TILE_SIZE = 64;

enum class Tile
{
    Empty = 0,
    Wall
};

constexpr int LEVEL_WIDTH = 5;
constexpr int LEVEL_HEIGHT = 5;

Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Wall,  Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall},
};

// TODO: rename Tile_Texture -> Sprite
struct Tile_Texture
{
    SDL_Rect srcrect;
    SDL_Texture *texture;
};

void render_tile_texture(SDL_Renderer *renderer,
                         Tile_Texture texture,
                         SDL_Rect destrect)
{
    sdl(SDL_RenderCopy(
            renderer,
            texture.texture,
            &texture.srcrect,
            &destrect));
}

void render_level(SDL_Renderer *renderer, Tile_Texture wall_texture)
{
    for (int y = 0; y < LEVEL_HEIGHT; ++y) {
        for (int x = 0; x < LEVEL_WIDTH; ++x) {
            switch (level[y][x]) {
            case Tile::Empty: {
            } break;

            case Tile::Wall: {
                sdl(SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255));
                render_tile_texture(renderer, wall_texture,
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
    uint32_t *image_pixels = (uint32_t *) std::malloc(sizeof(uint32_t) * image.width * image.height);

    if (!png_image_finish_read(&image, nullptr, image_pixels, 0, nullptr)) {
        fprintf(stderr, "libpng pooped itself: %s\n", image.message);
        abort();
    }

    SDL_Surface* image_surface =
        sdl(SDL_CreateRGBSurfaceFrom(image_pixels,
                                     image.width,
                                     image.height,
                                     32,
                                     image.width * 4,
                                     0x000000FF,
                                     0x0000FF00,
                                     0x00FF0000,
                                     0xFF000000));

    SDL_Texture *image_texture =
        sdl(SDL_CreateTextureFromSurface(renderer,
                                         image_surface));
    SDL_FreeSurface(image_surface);

    return image_texture;
}

int main(void)
{
    sdl(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window =
        sdl(SDL_CreateWindow(
                "New folder 1",
                0, 0, 800, 600,
                SDL_WINDOW_RESIZABLE));

    SDL_Renderer *renderer =
        sdl(SDL_CreateRenderer(
                window, -1,
                SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

    // TODO(#8): replace fantasy_tiles.png with our own assets
    SDL_Texture *tileset_texture = load_texture_from_png_file(
        renderer,
        "fantasy_tiles.png");

    Tile_Texture wall_texture = {
        {120, 128, 16, 16},
        tileset_texture
    };

    // TODO(#9): baking assets into executable
    SDL_Texture *walking_texture = load_texture_from_png_file(
        renderer,
        "walking-12px-zoom.png");

    constexpr int walking_frame_size = 48;
    constexpr int walking_frame_count = 4;
    constexpr int walking_frame_duration = 200;
    int walking_frame_current = 0;
    Tile_Texture walking_frames[walking_frame_count];

    for (int i = 0; i < walking_frame_count; ++i) {
        walking_frames[i].srcrect = {
            i * walking_frame_size,
            0,
            walking_frame_size,
            walking_frame_size
        };
        walking_frames[i].texture = walking_texture;
    }

    Uint32 walking_frame_cooldown = walking_frame_duration;

    int x = 0;

    bool quit = false;
    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
    while (!quit) {
        const Uint32 begin = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                quit = true;
            } break;
            }
        }

        if (keyboard[SDL_SCANCODE_D]) {
            x += 1;
        } else if (keyboard[SDL_SCANCODE_A]) {
            x -= 1;
        }

        sdl(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
        sdl(SDL_RenderClear(renderer));

        render_level(renderer, wall_texture);
        render_tile_texture(renderer, walking_frames[walking_frame_current],
                            {x, 4 * TILE_SIZE - walking_frame_size,
                             walking_frame_size, walking_frame_size});

        SDL_RenderPresent(renderer);

        const Uint32 dt = SDL_GetTicks() - begin;

        if (dt < walking_frame_cooldown) {
            walking_frame_cooldown -= dt;
        } else {
            walking_frame_current = (walking_frame_current + 1) % walking_frame_count;
            walking_frame_cooldown = walking_frame_duration;
        }
    }
    SDL_Quit();

    return 0;
}
