
#pragma once

#include "GW_ROM/ball.h"
extern const GW_rom ball;
#include "GW_ROM/zelda.h"
extern const GW_rom zelda;




const GW_rom* GW_list[] = {&ball, &zelda};
const size_t nb_games = 2;

