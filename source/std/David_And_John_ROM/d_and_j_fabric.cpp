
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "d_and_j_fabric.h"


const uint8_t rom_GW_d_and_j_fabric[] = {
    0xFF, 0xFF, 0xFF, 0x04
}; const size_t size_rom_GW_d_and_j_fabric = sizeof(rom_GW_d_and_j_fabric)/sizeof(rom_GW_d_and_j_fabric[0]);

const uint8_t melody_GW_d_and_j_fabric[1] = {0}; 
	const size_t size_melody_GW_d_and_j_fabric = 0;

const std::string path_segment_d_and_j_fabric = "romfs:/gfx/segment_d_and_j_fabric.t3x"; // Visual of segment -> Big unique texture
const Segment segment_GW_d_and_j_fabric[] = {
	{ { 0,1,0 }, { 43,49 }, { 1,594 }, { 60,47 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 51,50 }, { 1,496 }, { 58,46 }, 0, 0, false, false, 0 }, { { 0,1,10 }, { 35,49 }, { 95,664 }, { 68,47 }, 0, 0, false, false, 0 }, { { 0,1,11 }, { 51,49 }, { 189,742 }, { 66,47 }, 0, 0, false, false, 0 }, { { 0,1,12 }, { 34,117 }, { 165,665 }, { 69,46 }, 0, 0, false, false, 0 }, { { 0,1,13 }, { 51,116 }, { 165,616 }, { 65,47 }, 0, 0, false, false, 0 }, { { 0,1,14 }, { 35,182 }, { 1,891 }, { 68,48 }, 0, 0, false, false, 0 }, { { 0,1,15 }, { 51,184 }, { 95,616 }, { 66,46 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 42,117 }, { 63,568 }, { 61,46 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 51,116 }, { 188,567 }, { 58,47 }, 0, 0, false, false, 0 }, { { 0,1,4 }, { 45,182 }, { 1,544 }, { 58,48 }, 0, 0, false, false, 0 }, { { 0,1,5 }, { 51,184 }, { 126,568 }, { 60,46 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 16,78 }, { 63,624 }, { 22,17 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 118,65 }, { 209,966 }, { 36,30 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 16,145 }, { 1,453 }, { 22,17 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 118,132 }, { 209,934 }, { 36,30 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 16,212 }, { 25,454 }, { 22,16 }, 0, 0, false, false, 0 }, { { 0,2,5 }, { 118,198 }, { 205,902 }, { 36,30 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 0,69 }, { 237,463 }, { 15,26 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 152,66 }, { 236,682 }, { 18,29 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 0,134 }, { 232,844 }, { 15,28 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 155,133 }, { 203,471 }, { 15,29 }, 0, 0, false, false, 0 }, { { 0,3,4 }, { 0,201 }, { 220,462 }, { 15,27 }, 0, 0, false, false, 0 }, { { 0,3,5 }, { 155,202 }, { 237,435 }, { 15,26 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 0,96 }, { 1,484 }, { 44,10 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 106,96 }, { 132,473 }, { 64,10 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 0,162 }, { 61,507 }, { 44,10 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 106,162 }, { 1,472 }, { 64,10 }, 0, 0, false, false, 0 }, { { 0,4,4 }, { 0,229 }, { 199,799 }, { 44,10 }, 0, 0, false, false, 0 }, { { 0,4,5 }, { 106,229 }, { 67,461 }, { 64,10 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 51,21 }, { 225,713 }, { 27,27 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 27,21 }, { 199,811 }, { 27,27 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 4,21 }, { 228,811 }, { 26,27 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 43,5 }, { 203,521 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 43,5 }, { 247,669 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 43,5 }, { 243,921 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 43,5 }, { 243,908 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 43,5 }, { 107,506 }, { 11,11 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 43,5 }, { 243,895 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 43,5 }, { 236,669 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 43,5 }, { 241,882 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 43,5 }, { 49,459 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 43,5 }, { 133,460 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 33,5 }, { 144,460 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 33,5 }, { 248,605 }, { 7,11 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 33,5 }, { 155,460 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 33,5 }, { 166,460 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 33,5 }, { 245,798 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 33,5 }, { 177,460 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 33,5 }, { 188,460 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 33,5 }, { 199,458 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 33,5 }, { 210,449 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 33,5 }, { 221,449 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 22,5 }, { 60,448 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 22,5 }, { 247,985 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 22,5 }, { 49,446 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 22,5 }, { 25,441 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 22,5 }, { 179,521 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 22,5 }, { 36,441 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 22,5 }, { 1,440 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 22,5 }, { 12,440 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 22,5 }, { 71,448 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 22,5 }, { 82,448 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 11,5 }, { 93,448 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 11,5 }, { 247,972 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 11,5 }, { 104,448 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 12,5 }, { 247,959 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 12,5 }, { 191,521 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 12,5 }, { 247,946 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 12,5 }, { 247,521 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 12,5 }, { 247,508 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 11,5 }, { 115,448 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 11,5 }, { 126,447 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,0,0 }, { 57,214 }, { 209,998 }, { 45,25 }, 0, 1, false, false, 0 }, { { 0,0,1 }, { 129,208 }, { 179,534 }, { 68,31 }, 0, 1, false, false, 0 }, { { 0,1,16 }, { 57,14 }, { 1,795 }, { 102,74 }, 0, 1, false, false, 0 }, { { 0,1,17 }, { 80,12 }, { 103,871 }, { 100,76 }, 0, 1, false, false, 0 }, { { 0,1,18 }, { 55,117 }, { 103,949 }, { 104,74 }, 0, 1, false, false, 0 }, { { 0,1,19 }, { 80,109 }, { 1,941 }, { 100,82 }, 0, 1, false, false, 0 }, { { 0,1,6 }, { 65,14 }, { 1,719 }, { 94,74 }, 0, 1, false, false, 0 }, { { 0,1,7 }, { 80,12 }, { 97,713 }, { 90,76 }, 0, 1, false, false, 0 }, { { 0,1,8 }, { 67,117 }, { 1,643 }, { 92,74 }, 0, 1, false, false, 0 }, { { 0,1,9 }, { 80,113 }, { 105,791 }, { 92,78 }, 0, 1, false, false, 0 }, { { 0,2,6 }, { 24,59 }, { 189,714 }, { 34,26 }, 0, 1, false, false, 0 }, { { 0,2,7 }, { 183,39 }, { 120,520 }, { 57,46 }, 0, 1, false, false, 0 }, { { 0,2,8 }, { 24,163 }, { 205,874 }, { 34,26 }, 0, 1, false, false, 0 }, { { 0,2,9 }, { 183,142 }, { 61,519 }, { 57,47 }, 0, 1, false, false, 0 }, { { 0,3,6 }, { 0,43 }, { 71,897 }, { 23,42 }, 0, 1, false, false, 0 }, { { 0,3,7 }, { 239,53 }, { 205,840 }, { 25,32 }, 0, 1, false, false, 0 }, { { 0,3,8 }, { 0,148 }, { 222,491 }, { 23,41 }, 0, 1, false, false, 0 }, { { 0,3,9 }, { 241,138 }, { 232,618 }, { 23,45 }, 0, 1, false, false, 0 }, { { 0,4,6 }, { 0,86 }, { 61,484 }, { 69,16 }, 0, 1, false, false, 0 }, { { 0,4,7 }, { 164,86 }, { 1,873 }, { 100,16 }, 0, 1, false, false, 0 }, { { 0,4,8 }, { 0,190 }, { 132,485 }, { 69,15 }, 0, 1, false, false, 0 }, { { 0,4,9 }, { 164,190 }, { 120,502 }, { 100,16 }, 0, 1, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_fabric = sizeof(segment_GW_d_and_j_fabric)/sizeof(segment_GW_d_and_j_fabric[0]); 
 const uint16_t segment_info_d_and_j_fabric[] = { 256, 1024, 1, 0, 170, 240, 264, 240}; 

const std::string path_background_d_and_j_fabric = "";
const uint16_t background_info_d_and_j_fabric[] = { 512, 512, 0, 0, 170, 240, 0, 0, 264, 240, 1, 0, 0 }; 



const std::string path_console_d_and_j_fabric = "romfs:/gfx/console_d_and_j_fabric.t3x";
const uint16_t console_info_d_and_j_fabric[] = { 512, 256, 0, 16, 320, 240}; 




const GW_rom d_and_j_fabric (
    "New Job for John", "D_AND_J_FABRIC", "2024-08-26"
    , rom_GW_d_and_j_fabric, size_rom_GW_d_and_j_fabric
    , melody_GW_d_and_j_fabric, size_melody_GW_d_and_j_fabric
    , path_segment_d_and_j_fabric
    , segment_GW_d_and_j_fabric, size_segment_GW_d_and_j_fabric
    , segment_info_d_and_j_fabric
    , path_background_d_and_j_fabric
    , background_info_d_and_j_fabric
    , path_console_d_and_j_fabric
    , console_info_d_and_j_fabric
    , GW_rom::MANUFACTURER_DAVID_AND_JOHN
);

