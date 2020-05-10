struct Sample_S16
{
    int16_t* audio_buf;
    Uint32 audio_len;
    Uint32 audio_cur;
};

const size_t SAMPLE_MIXER_CAPACITY = 5;

struct Sample_Mixer
{
    float volume;
    Sample_S16 samples[SAMPLE_MIXER_CAPACITY];

    void play_sample(Sample_S16 sample)
    {
        for (size_t i = 0; i < SAMPLE_MIXER_CAPACITY; ++i) {
            if (samples[i].audio_cur >= samples[i].audio_len) {
                samples[i] = sample;
                samples[i].audio_cur = 0;
                return;
            }
        }
    }
};

const size_t SOMETHING_SOUND_FREQ = 48000;
const size_t SOMETHING_SOUND_FORMAT = 32784;
const size_t SOMETHING_SOUND_CHANNELS = 1;
const size_t SOMETHING_SOUND_SAMPLES = 4096;

struct Sample_S16_File
{
    const char *file_path;
    Sample_S16 sample;
};

Sample_S16_File sample_s16_files[] = {
    {"./assets/sounds/enemy_shoot-48000-decay.wav", {}},
    {"./assets/sounds/jumppp11-48000-mono.wav", {}},
    {"./assets/sounds/jumppp22-48000-mono.wav", {}},
};
const size_t sample_s16_files_count = sizeof(sample_s16_files) / sizeof(sample_s16_files[0]);

Sample_S16 sample_s16_by_name(String_View file_path)
{
    for (size_t i = 0; i < sample_s16_files_count; ++i) {
        if (file_path == cstr_as_string_view(sample_s16_files[i].file_path)) {
            return sample_s16_files[i].sample;
        }
    }

    return {};
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

void sample_mixer_audio_callback(void *userdata, Uint8 *stream, int len)
{
    Sample_Mixer *mixer = (Sample_Mixer *)userdata;

    int16_t *output = (int16_t *)stream;
    size_t output_len = (size_t) len / sizeof(*output);

    memset(stream, 0, (size_t) len);
    for (size_t i = 0; i < SAMPLE_MIXER_CAPACITY; ++i) {
        for (size_t j = 0; j < output_len; ++j) {
            int16_t x = 0;

            if (mixer->samples[i].audio_cur < mixer->samples[i].audio_len) {
                x = mixer->samples[i].audio_buf[mixer->samples[i].audio_cur];
                mixer->samples[i].audio_cur += 1;
            }

            output[j] = (int16_t) clamp(output[j] + x, (int) INT16_MIN, (int) INT16_MAX);
        }
    }

    for (size_t i = 0; i < output_len; ++i) {
        output[i] = (int16_t) (output[i] * mixer->volume);
    }
}

void load_samples()
{
    for (size_t i = 0; i < sample_s16_files_count; ++i) {
        sample_s16_files[i].sample = load_wav_as_sample_s16(sample_s16_files[i].file_path);
    }
}
