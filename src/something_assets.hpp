#ifndef SOMETHING_ASSETS_HPP_
#define SOMETHING_ASSETS_HPP_

#include "./something_sound.hpp"

const size_t ASSETS_CONF_BUFFER_CAPACITY = 1024 * 1024;
const size_t ASSETS_TEXTURES_CAPACITY = 128;
const size_t ASSETS_SOUNDS_CAPACITY = 128;
const size_t ASSETS_ANIMATS_CAPACITY = 128;

template <typename T>
struct Asset
{
    String_View id;
    String_View path;
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
    bool loaded_first_time;

    char conf_buffer[ASSETS_CONF_BUFFER_CAPACITY];
    size_t textures_count;
    Asset<Texture> textures[ASSETS_TEXTURES_CAPACITY];
    size_t sounds_count;
    Asset<Sample_S16> sounds[ASSETS_SOUNDS_CAPACITY];
    size_t animats_count;
    Asset<Frame_Animat> animats[ASSETS_ANIMATS_CAPACITY];

    Maybe<Texture_Index> get_texture_by_id(String_View id);
    Texture_Index get_texture_by_id_or_panic(String_View id);

    Maybe<Sample_S16> get_sound_by_id(String_View id);
    Sample_S16 get_sound_by_id_or_panic(String_View id);

    Maybe<Frame_Animat> get_animat_by_id(String_View id);
    Frame_Animat get_animat_by_id_or_panic(String_View id);

    String_View load_file_into_conf_buffer(const char *filepath);
    void load_texture(SDL_Renderer *renderer, String_View id, String_View path);
    void load_sound(String_View id, String_View path);
    void load_animat(String_View id, String_View path);

    void clean();
    void load_conf(SDL_Renderer *renderer, const char *filepath);
};

extern Assets assets;

#endif  // SOMETHING_ASSETS_HPP_
