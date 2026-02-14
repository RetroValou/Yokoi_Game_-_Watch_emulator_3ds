
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "d_and_j_avignon.h"


const uint8_t rom_GW_d_and_j_avignon[] = {
    0xFF, 0xFF, 0xFF, 0x02
}; const size_t size_rom_GW_d_and_j_avignon = sizeof(rom_GW_d_and_j_avignon)/sizeof(rom_GW_d_and_j_avignon[0]);

const uint8_t melody_GW_d_and_j_avignon[1] = {0}; 
	const size_t size_melody_GW_d_and_j_avignon = 0;

const std::string path_segment_d_and_j_avignon = "romfs:/gfx/segment_d_and_j_avignon.t3x"; // Visual of segment -> Big unique texture
const Segment segment_GW_d_and_j_avignon[] = {
	{ { 0,0,0 }, { 273,5 }, { 1,901 }, { 44,56 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 256,52 }, { 69,585 }, { 30,61 }, 0, 0, false, false, 0 }, { { 0,0,10 }, { 53,53 }, { 1,839 }, { 40,60 }, 0, 0, false, false, 0 }, { { 0,0,11 }, { 81,127 }, { 47,861 }, { 54,45 }, 0, 0, false, false, 0 }, { { 0,0,12 }, { 14,12 }, { 48,715 }, { 44,54 }, 0, 0, false, false, 0 }, { { 0,0,13 }, { 0,53 }, { 68,523 }, { 29,60 }, 0, 0, false, false, 0 }, { { 0,0,14 }, { 6,138 }, { 1,617 }, { 60,34 }, 0, 0, false, false, 0 }, { { 0,0,15 }, { 320,55 }, { 94,648 }, { 32,58 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 271,127 }, { 43,814 }, { 53,45 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 205,13 }, { 1,759 }, { 45,53 }, 0, 0, false, false, 0 }, { { 0,0,4 }, { 185,52 }, { 94,708 }, { 33,61 }, 0, 0, false, false, 0 }, { { 0,0,5 }, { 205,131 }, { 48,771 }, { 58,41 }, 0, 0, false, false, 0 }, { { 0,0,6 }, { 142,10 }, { 65,908 }, { 45,55 }, 0, 0, false, false, 0 }, { { 0,0,7 }, { 123,53 }, { 56,653 }, { 36,60 }, 0, 0, false, false, 0 }, { { 0,0,8 }, { 139,130 }, { 1,671 }, { 53,42 }, 0, 0, false, false, 0 }, { { 0,0,9 }, { 81,8 }, { 65,965 }, { 44,58 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 276,114 }, { 1,438 }, { 64,22 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 276,114 }, { 1,414 }, { 64,22 }, 0, 0, false, false, 0 }, { { 0,1,10 }, { 83,114 }, { 1,390 }, { 64,22 }, 0, 0, false, false, 0 }, { { 0,1,11 }, { 81,107 }, { 1,586 }, { 66,29 }, 0, 0, false, false, 0 }, { { 0,1,12 }, { 18,114 }, { 1,366 }, { 64,22 }, 0, 0, false, false, 0 }, { { 0,1,13 }, { 18,114 }, { 1,342 }, { 64,22 }, 0, 0, false, false, 0 }, { { 0,1,14 }, { 17,107 }, { 1,555 }, { 65,29 }, 0, 0, false, false, 0 }, { { 0,1,15 }, { 0,114 }, { 108,884 }, { 18,22 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 275,107 }, { 1,524 }, { 65,29 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 211,114 }, { 1,318 }, { 64,22 }, 0, 0, false, false, 0 }, { { 0,1,4 }, { 211,114 }, { 1,294 }, { 64,22 }, 0, 0, false, false, 0 }, { { 0,1,5 }, { 210,107 }, { 1,493 }, { 65,29 }, 0, 0, false, false, 0 }, { { 0,1,6 }, { 147,114 }, { 1,270 }, { 64,22 }, 0, 0, false, false, 0 }, { { 0,1,7 }, { 147,114 }, { 1,246 }, { 64,22 }, 0, 0, false, false, 0 }, { { 0,1,8 }, { 146,107 }, { 1,462 }, { 65,29 }, 0, 0, false, false, 0 }, { { 0,1,9 }, { 83,114 }, { 1,222 }, { 64,22 }, 0, 0, false, false, 0 }, { { 0,10,0 }, { 335,14 }, { 1,959 }, { 62,64 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 313,35 }, { 108,864 }, { 19,18 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 249,35 }, { 108,824 }, { 18,18 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 185,35 }, { 108,804 }, { 18,18 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 120,35 }, { 108,844 }, { 19,18 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 56,35 }, { 108,784 }, { 18,18 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 359,80 }, { 1,721 }, { 38,36 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 343,0 }, { 68,488 }, { 38,33 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 323,142 }, { 101,618 }, { 26,28 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 347,142 }, { 101,588 }, { 26,28 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 371,142 }, { 101,558 }, { 26,28 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 384,126 }, { 1,657 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 384,126 }, { 119,545 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 384,126 }, { 12,657 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 384,126 }, { 1,826 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 384,126 }, { 108,771 }, { 11,11 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 385,126 }, { 119,517 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 385,126 }, { 98,847 }, { 8,12 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 385,126 }, { 119,504 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 384,126 }, { 23,657 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 384,126 }, { 34,657 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 374,126 }, { 45,657 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 374,126 }, { 34,825 }, { 7,12 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 374,126 }, { 112,972 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 374,126 }, { 12,826 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 374,126 }, { 112,999 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 374,126 }, { 23,826 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 374,126 }, { 112,958 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 374,126 }, { 68,475 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 374,126 }, { 112,944 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 374,126 }, { 112,930 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 363,126 }, { 112,916 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 363,126 }, { 98,833 }, { 8,12 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 363,126 }, { 47,945 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 363,126 }, { 79,475 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 363,126 }, { 112,986 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 363,126 }, { 90,475 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 363,126 }, { 47,931 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 363,126 }, { 101,475 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 363,126 }, { 47,917 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 363,126 }, { 108,544 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 352,126 }, { 99,530 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 352,126 }, { 98,819 }, { 8,12 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 352,126 }, { 110,530 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 352,126 }, { 112,475 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 352,126 }, { 112,1012 }, { 11,11 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 352,126 }, { 68,462 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 352,126 }, { 108,516 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 352,126 }, { 79,462 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 352,126 }, { 108,502 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 352,126 }, { 108,488 }, { 9,12 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_avignon = sizeof(segment_GW_d_and_j_avignon)/sizeof(segment_GW_d_and_j_avignon[0]); 
 const uint16_t segment_info_d_and_j_avignon[] = { 128, 1024, 1, 0, 397, 172}; 


const std::string path_background_d_and_j_avignon = "";
const uint16_t background_info_d_and_j_avignon[] = { 512, 256, 0, 0, 397, 172, 1, 0, 0 }; 


const std::string path_console_d_and_j_avignon = "romfs:/gfx/console_d_and_j_avignon.t3x";
const uint16_t console_info_d_and_j_avignon[] = { 512, 256, 0, 16, 320, 240}; 




const GW_rom d_and_j_avignon (
    "Vacation in Avignon", "D_AND_J_AVIGNON", "2024-08-26"
    , rom_GW_d_and_j_avignon, size_rom_GW_d_and_j_avignon
    , melody_GW_d_and_j_avignon, size_melody_GW_d_and_j_avignon
    , path_segment_d_and_j_avignon
    , segment_GW_d_and_j_avignon, size_segment_GW_d_and_j_avignon
    , segment_info_d_and_j_avignon
    , path_background_d_and_j_avignon
    , background_info_d_and_j_avignon
    , path_console_d_and_j_avignon
    , console_info_d_and_j_avignon
    , GW_rom::MANUFACTURER_DAVID_AND_JOHN
);

