#ifndef SOMETHING_ASSETS_HPP_
#define SOMETHING_ASSETS_HPP_

const size_t ASSETS_CONF_BUFFER_CAPACITY = 1024 * 1024;
const size_t ASSETS_TEXTURES_CAPACITY = 128;
const size_t ASSETS_SOUNDS_CAPACITY = 128;

template <typename T>
struct Asset
{
    String_View id;
    T unwrap;
};

struct Texture
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Surface *surface_mask;
    SDL_Texture *texture_mask;
};

struct Assets
{
    char conf_buffer[ASSETS_CONF_BUFFER_CAPACITY];
    size_t textures_count;
    Asset<Texture> textures[ASSETS_TEXTURES_CAPACITY];
    size_t sounds_count;
    Asset<Sample_S16> sounds[ASSETS_SOUNDS_CAPACITY];

    String_View load_file_into_conf_buffer(const char *filepath);
    void load_texture(SDL_Renderer *renderer, String_View id, String_View path);
    void load_sound(String_View id, String_View path);
    void load_animat(String_View id, String_View path);

    void load_conf(SDL_Renderer *renderer, const char *filepath);
};

#endif  // SOMETHING_ASSETS_HPP_
