#pragma once

#include <SDL.h>
#include <utility>

namespace yokoi
{
    using PointerId = std::pair<SDL_TouchID, SDL_FingerID>;
    enum class PointerPhase { press, move, release };
}
