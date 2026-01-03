#ifndef TIME_ADDRESSES_H
#define TIME_ADDRESSES_H
#include <stdint.h>
#include <string>

#include <unordered_map>

struct TimeAddress {
    uint8_t col_hour_tens,    line_hour_tens;
    uint8_t col_hour_units,   line_hour_units;
    uint8_t col_minute_tens,  line_minute_tens;
    uint8_t col_minute_units, line_minute_units;
    uint8_t col_second_tens,  line_second_tens;
    uint8_t col_second_units, line_second_units;
    uint8_t pm_bit;                                 // PM bit for 12-hour format
};

// Some of the address mappings in this file are derived from https://github.com/bzhxx/LCD-Game-Shrinker
// The rest came from inspection of ram dumps using debug_dump_ram_state() 

inline const TimeAddress* get_time_addresses(const std::string& ref_game) {
    // --- SM5A ---
    static const std::unordered_map<std::string, TimeAddress> sm5a_map = {
        {"AC_01", {4,8, 4,9, 4,10, 4,11, 99,99, 99,99, 0}}, // Ball
        {"FL_02", {4,4, 4,5, 4,6, 4,7, 99,99, 99,99, 0}}, // Flagman
        {"MT_03", {4,0, 4,1, 4,2, 4,3, 99,99, 99,99, 0}}, // Vermin
        {"RC_04", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Fire (Silver)
        {"MH_06", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Manhole (Gold)
        {"CN_07", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Helmet (Rev.1 / CN-07 original)
        {"CN_17", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Helmet (Rev.2 / CN-17 revised)
        {"LN_08", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Lion
        {"PR_21", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Parachute
        {"OC_22", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Octopus
        {"PP_23", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Popeye (Wide Screen)
        {"IP_05", {1,2, 1,3, 1,6, 1,7, 1,4, 1,5, 0}}, // Judge (Green / Original)
        {"IP_15", {1,2, 1,3, 1,6, 1,7, 1,4, 1,5, 0}}, // Judge (Purple / Revised)
        {"FP_24", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Chef
        {"MC_25", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Mickey Mouse (Wide Screen)
        {"EG_26", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Egg
        {"FR_27", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Fire (Wide Screen)
    };

    // --- SM510 ---
    static const std::unordered_map<std::string, TimeAddress> sm510_map = {
        {"TL_28", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Turtle Bridge
        {"ID_29", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Fire Attack
        {"SP_30", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Snoopy Tennis
        {"OP_51", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Oil Panic
        {"GH_54", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Green House
        {"MW_56", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mario Bros.
        {"LP_57", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Rain Shower
        {"MG_61", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Squish
        {"DJ_101", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Donkey Kong Jr. (New Wide Screen)
        {"ML_102", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mario's Cement Factory (New Wide Screen)
        {"TF_104", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Tropical Fish
        {"UD_202", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Crab Grab
        {"SM_91", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Snoopy (Panorama Screen)
        {"PG_92", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Popeye (Panorama Screen)
        {"CJ_93", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Donkey Kong Jr. (Panorama Screen)
        {"TB_94", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mario's Bombs Away
        {"DC_95", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Donkey Kong Circus
        {"MK_96", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mickey Mouse (Panorama Screen)
        {"CM_72", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mario's Cement Factory (Table Top, CM-72)
        {"CM_72A", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mario's Cement Factory (Table Top, CM-72A)
        {"DK_52", {4,5, 4,4, 4,3, 4,2, 4,1, 4,0, 2}}, // Donkey Kong
        {"DM_53", {4,9, 4,10, 4,11, 4,12, 4,7, 4,8, 2}}, // Mickey & Donald
        {"JR_55", {4,4, 4,3, 4,2, 4,1, 4,6, 4,5, 2}}, // Donkey Kong II
        {"TC_58", {0,10, 0,11, 0,12, 0,13, 0,14, 0,15, 8}}, // Life Boat
        {"NH_103", {0,10, 0,11, 0,12, 0,13, 0,14, 0,15, 8}}, // Manhole (New Wide Screen)
        {"BU_201", {0,10, 0,11, 0,12, 0,13, 0,14, 0,15, 8}}, // Spitball Sparky
        {"MG_8", {3,0, 3,1, 3,3, 3,4, 3,6, 3,7, 2}}, // Shuttle Voyage
    };

    // --- SM511/SM512 ---
    static const std::unordered_map<std::string, TimeAddress> sm511_map = {
        {"PB_59", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Pinball
        {"BJ_60", {0,5, 0,4, 0,3, 0,2, 0,1, 0,0, 24}}, // Black Jack (Only game with 24hr clock)
        {"BD_62", {1,0, 1,1, 1,2, 1,3, 1,4, 1,5, 2}}, // Bomb Sweeper
        {"JB_63", {1,0, 1,1, 1,2, 1,3, 1,4, 1,5, 2}}, // Safe Buster
        {"MV_64", {1,0, 1,1, 1,2, 1,3, 1,4, 1,5, 2}}, // Gold Cliff
        {"ZL_65", {1,3, 1,2, 1,1, 1,0, 1,5, 1,4, 2}}, // Zelda
        {"YM_801", {2,2, 2,3, 2,4, 2,5, 2,6, 2,7, 2}}, // Super Mario Bros. (Crystal Screen)
        {"YM_105", {2,2, 2,3, 2,4, 2,5, 2,6, 2,7, 2}}, // Super Mario Bros. (New Wide Screen)
        {"DR_802", {2,3, 2,4, 2,5, 2,6, 2,7, 2,8, 2}}, // Climber (Crystal Screen)
        {"DR_106", {2,3, 2,4, 2,5, 2,6, 2,7, 2,8, 2}}, // Climber (New Wide Screen)
        {"BF_803", {2,3, 2,4, 2,5, 2,6, 2,1, 2,2, 2}}, // Balloon Fight (Crystal Screen)
        {"BF_107", {2,3, 2,4, 2,5, 2,6, 2,1, 2,2, 2}}, // Balloon Fight (New Wide Screen)
        {"MB_108", {1,8, 1,9, 1,10, 1,11, 1,12, 1,13, 2}}, // Mario The Juggler
        {"BX_301", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Micro Vs. System: Boxing
        {"AK_302", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Micro Vs. System: Donkey Kong 3
        {"HK_303", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Micro Vs. System: Donkey Kong Hockey
    };

    auto it = sm5a_map.find(ref_game);
    if (it != sm5a_map.end()) return &it->second;
    it = sm510_map.find(ref_game);
    if (it != sm510_map.end()) return &it->second;
    it = sm511_map.find(ref_game);
    if (it != sm511_map.end()) return &it->second;
    return nullptr; // Return nullptr if no specific mapping exists
}

#endif // TIME_ADDRESSES_H
