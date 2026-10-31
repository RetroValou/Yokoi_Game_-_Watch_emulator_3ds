#pragma once

#include <SDL.h>
#include <stdexcept>
#include <string>

namespace yokoi
{
    inline void require_sdl(bool ok, const char *message)
    {
        if (!ok)
            throw std::runtime_error(std::string(message) + ": " + SDL_GetError());
    }
}
