#include "input/yokoi_controller.h"

#include <cstdio>
#include "yokoi_input_mapping.h"

namespace yokoi
{
    void Controller::open()
    {
        if (device_)
            return;
        for (int i = 0; i < SDL_NumJoysticks(); ++i)
        {
            if (!SDL_IsGameController(i))
                continue;
            device_.reset(SDL_GameControllerOpen(i));
            if (device_)
                break;
            std::fprintf(stderr, "Controller open failed: %s\n", SDL_GetError());
        }
    }

    void Controller::handle_event(const SDL_Event &event)
    {
        if (event.type == SDL_CONTROLLERDEVICEADDED)
            open();
        if (event.type == SDL_CONTROLLERDEVICEREMOVED && device_ &&
            !SDL_GameControllerGetAttached(device_.get()))
        {
            device_.reset();
            open();
        }
    }

    ControllerState Controller::poll()
    {
        struct Binding
        {
            SDL_GameControllerButton button;
            SDL_Scancode key;
            uint32_t bit;
        };
        constexpr Binding bindings[] = {
            {SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_SCANCODE_UP, CTL_DPAD_UP},
            {SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_SCANCODE_DOWN, CTL_DPAD_DOWN},
            {SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_SCANCODE_LEFT, CTL_DPAD_LEFT},
            {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_SCANCODE_RIGHT, CTL_DPAD_RIGHT},
            {SDL_CONTROLLER_BUTTON_A, SDL_SCANCODE_Z, CTL_A},
            {SDL_CONTROLLER_BUTTON_B, SDL_SCANCODE_X, CTL_B},
            {SDL_CONTROLLER_BUTTON_X, SDL_SCANCODE_S, CTL_X},
            {SDL_CONTROLLER_BUTTON_Y, SDL_SCANCODE_A, CTL_Y},
            {SDL_CONTROLLER_BUTTON_START, SDL_SCANCODE_RETURN, CTL_START},
            {SDL_CONTROLLER_BUTTON_BACK, SDL_SCANCODE_TAB, CTL_SELECT},
            {SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_SCANCODE_Q, CTL_L1},
            {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_SCANCODE_R, CTL_MENU},
            {SDL_CONTROLLER_BUTTON_GUIDE, SDL_SCANCODE_F1, CTL_MENU},
        };
        const auto *keys = SDL_GetKeyboardState(nullptr);
        uint32_t mask = 0;
        for (const auto &binding : bindings)
        {
            if (keys[binding.key] ||
                (device_ && SDL_GameControllerGetButton(device_.get(), binding.button)))
                mask |= binding.bit;
        }
        if (device_)
        {
            const auto x = SDL_GameControllerGetAxis(device_.get(), SDL_CONTROLLER_AXIS_LEFTX);
            const auto y = SDL_GameControllerGetAxis(device_.get(), SDL_CONTROLLER_AXIS_LEFTY);
            if (x < -16000)
                mask |= CTL_DPAD_LEFT;
            if (x > 16000)
                mask |= CTL_DPAD_RIGHT;
            if (y < -16000)
                mask |= CTL_DPAD_UP;
            if (y > 16000)
                mask |= CTL_DPAD_DOWN;
        }
        // Menu navigation is edge-triggered; emulation consumes the held mask.
        const ControllerState state{mask, mask & ~previous_};
        previous_ = mask;
        return state;
    }
}
