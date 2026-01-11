#include "load_file.h"

#include "gw_pack.h"

#if !defined(YOKOI_EXTERNAL_ROMPACK_ONLY)
    #if defined(__ANDROID__)
        #include "GW_ALL_rgds.h"
    #else
        #include "GW_ALL.h"
    #endif
#endif


const GW_rom* load_game(uint8_t i_game){
    if (gw_pack::is_loaded()) {
        return gw_pack::game_at(i_game);
    }

#if !defined(YOKOI_EXTERNAL_ROMPACK_ONLY)
    if (i_game < nb_games) {
        return GW_list[i_game];
    }
#endif
    return nullptr;
}

std::string get_name(uint8_t i_game){
    const GW_rom* g = load_game(i_game);
    return g ? g->name : std::string();
}

size_t get_nb_name(){
    if (gw_pack::is_loaded()) {
        return gw_pack::game_count();
    }
#if !defined(YOKOI_EXTERNAL_ROMPACK_ONLY)
    return nb_games;
#else
    return 0;
#endif
}

std::string get_path_console_img(uint8_t i_game){
    const GW_rom* g = load_game(i_game);
    return g ? g->path_console : std::string();
}

const uint16_t* get_info_console_img(uint8_t i_game){
    const GW_rom* g = load_game(i_game);
    return g ? g->console_info : nullptr;
}

std::string get_date(uint8_t i_game){
    const GW_rom* g = load_game(i_game);
    return g ? g->date : std::string();
}

