#include "core/yokoi_audio.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include "SM5XX/SM5XX.h"

namespace yokoi
{
    Audio::~Audio()
    {
        if (device_)
            SDL_CloseAudioDevice(device_);
        SDL_FreeAudioStream(stream_);
    }

    bool Audio::open()
    {
        if (device_)
            return true;
        SDL_AudioSpec desired{};
        desired.freq = 48000;
        desired.format = AUDIO_S16SYS;
        desired.channels = 2;
        desired.samples = 1024;
        device_ = SDL_OpenAudioDevice(nullptr, 0, &desired, &output_,
                                     SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
        if (!device_)
        {
            std::fprintf(stderr, "Audio disabled: %s\n", SDL_GetError());
            return false;
        }
        std::fprintf(stderr, "Audio opened: %d Hz, %u channels\n",
                     output_.freq, unsigned(output_.channels));
        SDL_PauseAudioDevice(device_, 0);
        return true;
    }

    void Audio::configure(const SM5XX &cpu)
    {
        clear_queue();
        SDL_FreeAudioStream(stream_);
        stream_ = nullptr;
        const int source_rate = int(cpu.frequency / std::max(1u, cpu.sound_divide_frequency));
        if (device_)
        {
            stream_ = SDL_NewAudioStream(AUDIO_S16SYS, 1, source_rate,
                                        output_.format, output_.channels, output_.freq);
            if (!stream_)
                std::fprintf(stderr, "Audio conversion disabled: %s\n", SDL_GetError());
        }
    }

    void Audio::clear_queue()
    {
        if (device_)
            SDL_ClearQueuedAudio(device_);
    }

    void Audio::reset_sampling()
    {
        wait_ = 0;
        value_ = false;
    }

    void Audio::begin_frame()
    {
        samples_.clear();
    }

    void Audio::sample(SM5XX &cpu)
    {
        // Preserve any active sound pulse within the CPU's sample divider.
        value_ = value_ || cpu.get_active_sound();
        if (++wait_ >= std::max(1u, cpu.sound_divide_frequency))
        {
            samples_.push_back(value_ ? 26213 : 0);
            wait_ = 0;
            value_ = false;
        }
    }

    void Audio::submit_frame()
    {
        if (!stream_ || samples_.empty())
            return;
        if (SDL_AudioStreamPut(stream_, samples_.data(), int(samples_.size() * sizeof(int16_t))) < 0)
        {
            std::fprintf(stderr, "Audio conversion failed: %s\n", SDL_GetError());
            return;
        }
        // Drop a backlog above 200 ms rather than letting sound lag behind play.
        if (SDL_GetQueuedAudioSize(device_) > unsigned(output_.freq * output_.channels * 2 / 5))
            clear_queue();
        std::array<unsigned char, 16384> output;
        int bytes;
        while ((bytes = SDL_AudioStreamGet(stream_, output.data(), output.size())) > 0)
        {
            if (SDL_QueueAudio(device_, output.data(), bytes) < 0)
            {
                std::fprintf(stderr, "Audio queue failed: %s\n", SDL_GetError());
                return;
            }
        }
        if (bytes < 0)
            std::fprintf(stderr, "Audio conversion read failed: %s\n", SDL_GetError());
    }
}
