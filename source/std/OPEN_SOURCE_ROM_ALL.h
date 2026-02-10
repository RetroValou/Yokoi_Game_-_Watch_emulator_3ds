
#pragma once

#include "David_And_John_ROM/d_and_j_climber.h"
extern const GW_rom d_and_j_climber;

#include "David_And_John_ROM/d_and_j_avignon.h"
extern const GW_rom d_and_j_avignon;

#include "David_And_John_ROM/d_and_j_rythme.h"
extern const GW_rom d_and_j_rythme;

#include "David_And_John_ROM/d_and_j_fabric.h"
extern const GW_rom d_and_j_fabric;







const GW_rom* OPEN_SOURCE_list[] = {&d_and_j_climber, &d_and_j_avignon, &d_and_j_rythme, &d_and_j_fabric};
const size_t nb_games_OPEN_SOURCE = sizeof(OPEN_SOURCE_list) / sizeof(OPEN_SOURCE_list[0]);

