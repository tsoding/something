#include "./something_sound.hpp"

void Sample_Mixer::clean()
{
    for (size_t i = 0; i < SAMPLE_MIXER_CAPACITY; ++i) {
        slots[i].playing = false;
    }
}

void Sample_Mixer::play_sample(Index<Sample_S16> index)
{
    for (size_t i = 0; i < SAMPLE_MIXER_CAPACITY; ++i) {
        if (!slots[i].playing) {
            slots[i].cursor = 0;
            slots[i].index = index;
            slots[i].playing = true;
            return;
        }
    }
}

Sample_S16 load_wav_as_sample_s16(const char *file_path)
{
    Sample_S16 sample = {};
    SDL_AudioSpec want = {};
    if (SDL_LoadWAV(file_path, &want, (Uint8**) &sample.audio_buf, &sample.audio_len) == nullptr) {
        println(stderr, "SDL pooped itself: Failed to load ", file_path, ": ",
                SDL_GetError());
        abort();
    }

    assert(SDL_AUDIO_BITSIZE(want.format) == 16);
    assert(SDL_AUDIO_ISLITTLEENDIAN(want.format));
    assert(SDL_AUDIO_ISSIGNED(want.format));
    assert(SDL_AUDIO_ISINT(want.format));
    assert(want.freq == SOMETHING_SOUND_FREQ);
    assert(want.channels == SOMETHING_SOUND_CHANNELS);
    assert(want.samples == SOMETHING_SOUND_SAMPLES);

    sample.audio_len /= 2;

    return sample;
}

Sample_S16 load_wav_as_sample_s16(String_View file_path)
{
    char *filepath_cstr = (char*) malloc(file_path.count + 1);
    assert(filepath_cstr != NULL);
    memcpy(filepath_cstr, file_path.data, file_path.count);
    filepath_cstr[file_path.count] = '\0';
    auto result = load_wav_as_sample_s16(filepath_cstr);
    free(filepath_cstr);
    return result;
}

void sample_mixer_audio_callback(void *userdata, Uint8 *stream, int len)
{
    Sample_Mixer *mixer = (Sample_Mixer *)userdata;

    int16_t *output = (int16_t *)stream;
    size_t output_len = (size_t) len / sizeof(*output);

    memset(stream, 0, (size_t) len);
    for (size_t i = 0; i < SAMPLE_MIXER_CAPACITY; ++i) {
        if (mixer->slots[i].playing) {
            auto sample = assets.get_sound_by_index(mixer->slots[i].index);

            for (size_t j = 0; j < output_len; ++j) {
                int16_t x = 0;

                if (mixer->slots[i].cursor < sample.audio_len) {
                    x = sample.audio_buf[mixer->slots[i].cursor];
                    mixer->slots[i].cursor += 1;
                }

                output[j] = (int16_t) clamp(output[j] + x, (int) INT16_MIN, (int) INT16_MAX);
            }

            if (mixer->slots[i].cursor >= sample.audio_len) {
                mixer->slots[i].playing = false;
            }
        }
    }

    for (size_t i = 0; i < output_len; ++i) {
        output[i] = (int16_t) (output[i] * mixer->volume);
    }
}
