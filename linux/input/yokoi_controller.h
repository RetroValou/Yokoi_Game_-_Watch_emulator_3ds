#pragma once

#include <SDL.h>
#include <cstdint>
#include <memory>

namespace yokoi
{
    struct ControllerState
    {
        uint32_t held = 0;
        uint32_t pressed = 0;
    };

    // Keyboard and the first attached game controller share Android's CTL bits.
    class Controller
    {
    public:
        void open();
        void handle_event(const SDL_Event &event);
        ControllerState poll();

    private:
        std::unique_ptr<SDL_GameController, decltype(&SDL_GameControllerClose)>
            device_{nullptr, SDL_GameControllerClose};
        uint32_t previous_ = 0;
    };
}
