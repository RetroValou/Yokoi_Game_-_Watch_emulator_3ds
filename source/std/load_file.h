#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "segment.h"
#include "GW_ROM.h"

const GW_rom* load_game(uint8_t i_game);

std::string get_name(uint8_t i_game);
size_t get_nb_name();
std::string get_path_console_img(uint8_t i_game);
const uint16_t* get_info_console_img(uint8_t i_game);
std::string get_date(uint8_t i_game);

