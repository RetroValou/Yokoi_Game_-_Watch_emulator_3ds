#pragma once

#include <SDL.h>
#include <cstdint>
#include <vector>

class SM5XX;

namespace yokoi
{
    // Converts the CPU's one-bit sound into SDL's negotiated output format.
    class Audio
    {
    public:
        Audio() = default;
        ~Audio();
        Audio(const Audio &) = delete;
        Audio &operator=(const Audio &) = delete;

        bool open();
        void configure(const SM5XX &cpu);
        void clear_queue();
        void reset_sampling();
        void begin_frame();
        void sample(SM5XX &cpu);
        void submit_frame();
        bool has_stream() const { return stream_ != nullptr; }

    private:
        SDL_AudioDeviceID device_ = 0;
        SDL_AudioStream *stream_ = nullptr;
        SDL_AudioSpec output_{};
        uint32_t wait_ = 0;
        bool value_ = false;
        std::vector<int16_t> samples_;
    };
}
