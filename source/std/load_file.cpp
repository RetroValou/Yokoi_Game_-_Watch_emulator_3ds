#include "load_file.h"
#include "GW_ALL.h"


const GW_rom* load_game(uint8_t i_game){
    if(i_game < nb_games){ return GW_list[i_game]; }
    return nullptr;
}

std::string get_name(uint8_t i_game){ return GW_list[i_game]->name;}
size_t get_nb_name(){ return nb_games;}

std::string get_path_console_img(uint8_t i_game){ return GW_list[i_game]->path_console;}
const uint16_t* get_info_console_img(uint8_t i_game){ return GW_list[i_game]->console_info;}
std::string get_date(uint8_t i_game){ return GW_list[i_game]->date;}

