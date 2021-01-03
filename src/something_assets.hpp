#ifndef SOMETHING_ASSETS_HPP_
#define SOMETHING_ASSETS_HPP_

#include "./something_sound.hpp"
#include "./something_parsers.hpp"

const size_t ASSETS_CONF_BUFFER_CAPACITY = 1024 * 1024;
const size_t ASSETS_TEXTURES_CAPACITY = 128;
const size_t ASSETS_SOUNDS_CAPACITY = 128;
const size_t ASSETS_FRAMESEN_CAPACITY = 128;
const size_t ASSETS_SPRITES_CAPACITY = 128;

template <typename T>
struct Asset
{
    String_View id;
    String_View path;
    T unwrap;
};

struct Assets
{
    bool loaded_first_time;

    char conf_buffer[ASSETS_CONF_BUFFER_CAPACITY];
    size_t textures_count;
    Asset<Texture> textures[ASSETS_TEXTURES_CAPACITY];
    size_t sounds_count;
    Asset<Sample_S16> sounds[ASSETS_SOUNDS_CAPACITY];
    // NOTE: `framesen` is the plural of `frames` (and `frames` is the
    // plural of `frame`, yes).
    size_t framesen_count;
    Asset<Frames> framesen[ASSETS_FRAMESEN_CAPACITY];
    size_t sprite_count;
    Asset<Sprite> sprites[ASSETS_SPRITES_CAPACITY];

    Maybe<Index<Texture>> get_texture_by_id(String_View id);
    Texture get_texture_by_index(Index<Texture> index);

    Maybe<Index<Sample_S16>> get_sound_by_id(String_View id);
    Sample_S16 get_sound_by_index(Index<Sample_S16> index);

    Maybe<Index<Frames>> get_frames_by_id(String_View id);
    Frames get_frames_by_index(Index<Frames> index);

    Maybe<Index<Sprite>> get_sprite_by_id(String_View id);
    Sprite get_sprite_by_index(Index<Sprite>);

    String_View load_file_into_conf_buffer(const char *filepath);
    void load_texture(SDL_Renderer *renderer, String_View id, String_View path);
    void load_sound(String_View id, String_View path);
    void load_frames(String_View id, String_View path);

    void clean();
    void load_conf(SDL_Renderer *renderer, const char *filepath);
};

extern Assets assets;

#endif  // SOMETHING_ASSETS_HPP_
