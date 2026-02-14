
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "d_and_j_esquive.h"


const uint8_t rom_GW_d_and_j_esquive[] = {
    0xFF, 0xFF, 0xFF, 0x05
}; const size_t size_rom_GW_d_and_j_esquive = sizeof(rom_GW_d_and_j_esquive)/sizeof(rom_GW_d_and_j_esquive[0]);

const uint8_t melody_GW_d_and_j_esquive[1] = {0}; 
	const size_t size_melody_GW_d_and_j_esquive = 0;

const std::string path_segment_d_and_j_esquive = "romfs:/gfx/segment_d_and_j_esquive.t3x"; // Visual of segment -> Big unique texture
const Segment segment_GW_d_and_j_esquive[] = {
	{ { 0,0,0 }, { 212,10 }, { 1,390 }, { 39,31 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 216,60 }, { 1,582 }, { 35,40 }, 0, 0, false, false, 0 }, { { 0,0,10 }, { 4,57 }, { 1,699 }, { 38,40 }, 0, 0, false, false, 0 }, { { 0,0,11 }, { 8,108 }, { 1,540 }, { 34,40 }, 0, 0, false, false, 0 }, { { 0,0,12 }, { 363,39 }, { 1,961 }, { 34,62 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 210,110 }, { 1,359 }, { 39,29 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 148,16 }, { 1,501 }, { 34,37 }, 0, 0, false, false, 0 }, { { 0,0,4 }, { 142,59 }, { 1,662 }, { 41,35 }, 0, 0, false, false, 0 }, { { 0,0,5 }, { 147,110 }, { 1,462 }, { 34,37 }, 0, 0, false, false, 0 }, { { 0,0,6 }, { 72,10 }, { 1,741 }, { 40,39 }, 0, 0, false, false, 0 }, { { 0,0,7 }, { 79,63 }, { 1,423 }, { 33,37 }, 0, 0, false, false, 0 }, { { 0,0,8 }, { 71,111 }, { 1,624 }, { 39,36 }, 0, 0, false, false, 0 }, { { 0,0,9 }, { 1,10 }, { 1,782 }, { 36,44 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 283,20 }, { 1,130 }, { 30,17 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 283,69 }, { 38,606 }, { 25,16 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 287,114 }, { 42,411 }, { 21,19 }, 0, 0, false, false, 0 }, { { 0,10,0 }, { 315,5 }, { 33,12 }, { 2,135 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 253,9 }, { 1,324 }, { 32,33 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 252,64 }, { 34,189 }, { 27,26 }, 0, 0, false, false, 0 }, { { 0,2,10 }, { 44,57 }, { 1,219 }, { 31,33 }, 0, 0, false, false, 0 }, { { 0,2,11 }, { 42,112 }, { 35,330 }, { 28,27 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 253,106 }, { 1,289 }, { 32,33 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 182,16 }, { 35,273 }, { 28,26 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 183,57 }, { 1,254 }, { 32,33 }, 0, 0, false, false, 0 }, { { 0,2,5 }, { 182,112 }, { 35,301 }, { 28,27 }, 0, 0, false, false, 0 }, { { 0,2,6 }, { 114,9 }, { 1,184 }, { 31,33 }, 0, 0, false, false, 0 }, { { 0,2,7 }, { 112,64 }, { 35,245 }, { 28,26 }, 0, 0, false, false, 0 }, { { 0,2,8 }, { 114,106 }, { 1,149 }, { 31,33 }, 0, 0, false, false, 0 }, { { 0,2,9 }, { 42,16 }, { 34,217 }, { 28,26 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 301,20 }, { 37,961 }, { 3,2 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 301,69 }, { 39,798 }, { 3,3 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 301,114 }, { 39,794 }, { 3,2 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 285,16 }, { 39,811 }, { 24,6 }, 0, 0, false, false, 0 }, { { 0,3,4 }, { 285,65 }, { 39,819 }, { 24,7 }, 0, 0, false, false, 0 }, { { 0,3,5 }, { 285,110 }, { 39,803 }, { 24,6 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 323,4 }, { 1,872 }, { 41,42 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 323,49 }, { 1,916 }, { 41,43 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 323,95 }, { 1,828 }, { 41,42 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 317,142 }, { 36,432 }, { 27,28 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 342,142 }, { 37,995 }, { 26,28 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 366,142 }, { 37,965 }, { 26,28 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 301,153 }, { 44,738 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 301,153 }, { 55,738 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 301,153 }, { 44,725 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 301,153 }, { 44,712 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 301,153 }, { 44,790 }, { 11,11 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 301,153 }, { 41,699 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 301,153 }, { 52,699 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 301,153 }, { 44,686 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 301,153 }, { 44,673 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 301,153 }, { 44,660 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 291,153 }, { 55,725 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 291,153 }, { 56,777 }, { 7,11 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 291,153 }, { 44,952 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 291,153 }, { 44,939 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 291,153 }, { 44,777 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 291,153 }, { 44,926 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 291,153 }, { 44,913 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 291,153 }, { 55,712 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 291,153 }, { 44,900 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 291,153 }, { 55,686 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 280,153 }, { 44,887 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 280,153 }, { 56,764 }, { 7,11 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 280,153 }, { 44,874 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 280,153 }, { 44,861 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 280,153 }, { 44,764 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 280,153 }, { 44,848 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 280,153 }, { 44,835 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 280,153 }, { 42,398 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 280,153 }, { 53,398 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 280,153 }, { 42,385 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 269,153 }, { 53,385 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 269,153 }, { 55,673 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 269,153 }, { 42,372 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 269,153 }, { 42,359 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 269,153 }, { 44,751 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 269,153 }, { 53,372 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 269,153 }, { 53,359 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 269,153 }, { 42,647 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 269,153 }, { 53,647 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 269,153 }, { 42,634 }, { 9,11 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_esquive = sizeof(segment_GW_d_and_j_esquive)/sizeof(segment_GW_d_and_j_esquive[0]); 
 const uint16_t segment_info_d_and_j_esquive[] = { 64, 1024, 1, 0, 397, 172}; 

const std::string path_background_d_and_j_esquive = "";
const uint16_t background_info_d_and_j_esquive[] = { 512, 256, 0, 0, 397, 172, 1, 0, 0 }; 


const std::string path_console_d_and_j_esquive = "romfs:/gfx/console_d_and_j_esquive.t3x";
const uint16_t console_info_d_and_j_esquive[] = { 512, 256, 0, 16, 320, 240}; 




const GW_rom d_and_j_esquive (
    "Swimming assessment", "D_AND_J_ESQUIVE", "2024-08-26"
    , rom_GW_d_and_j_esquive, size_rom_GW_d_and_j_esquive
    , melody_GW_d_and_j_esquive, size_melody_GW_d_and_j_esquive
    , path_segment_d_and_j_esquive
    , segment_GW_d_and_j_esquive, size_segment_GW_d_and_j_esquive
    , segment_info_d_and_j_esquive
    , path_background_d_and_j_esquive
    , background_info_d_and_j_esquive
    , path_console_d_and_j_esquive
    , console_info_d_and_j_esquive
    , GW_rom::MANUFACTURER_DAVID_AND_JOHN
);

