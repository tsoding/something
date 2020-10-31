#ifndef SOMETHING_SOUND_HPP_
#define SOMETHING_SOUND_HPP_

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

    void play_sample(Sample_S16 sample);
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

Sample_S16 load_wav_as_sample_s16(const char *file_path);
Sample_S16 load_wav_as_sample_s16(String_View file_path);
void sample_mixer_audio_callback(void *userdata, Uint8 *stream, int len);

#endif  // SOMETHING_SOUND_HPP_
