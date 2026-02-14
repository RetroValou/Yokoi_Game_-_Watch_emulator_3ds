
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "d_and_j_climber.h"


const uint8_t rom_d_and_j_climber[] = {
    0xFF, 0xFF, 0xFF, 0x01
}; const size_t size_rom_d_and_j_climber = sizeof(rom_d_and_j_climber)/sizeof(rom_d_and_j_climber[0]);

const uint8_t melody_d_and_j_climber[1] = {0}; 
	const size_t size_melody_d_and_j_climber = 0;

const std::string path_segment_d_and_j_climber = "romfs:/gfx/segment_d_and_j_climber.t3x"; // Visual of segment -> Big unique texture
const Segment segment_d_and_j_climber[] ={
	{ { 0,0,0 }, { 6,93 }, { 68,596 }, { 51,61 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 63,97 }, { 46,242 }, { 45,56 }, 0, 0, false, false, 0 }, { { 0,0,12 }, { 99,21 }, { 93,242 }, { 30,56 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 119,97 }, { 50,416 }, { 52,56 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 0,178 }, { 52,300 }, { 47,56 }, 0, 0, false, false, 0 }, { { 0,0,4 }, { 64,178 }, { 50,474 }, { 52,58 }, 0, 0, false, false, 0 }, { { 0,0,5 }, { 118,178 }, { 52,358 }, { 48,56 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 0,85 }, { 1,729 }, { 60,73 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 57,87 }, { 68,800 }, { 58,75 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 114,101 }, { 68,659 }, { 58,60 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 0,179 }, { 73,889 }, { 54,61 }, 0, 0, false, false, 0 }, { { 0,1,4 }, { 53,169 }, { 1,877 }, { 67,71 }, 0, 0, false, false, 0 }, { { 0,1,5 }, { 107,169 }, { 1,804 }, { 65,71 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 24,78 }, { 44,159 }, { 32,21 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 65,78 }, { 78,159 }, { 32,21 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 133,78 }, { 1,100 }, { 31,21 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 12,160 }, { 69,113 }, { 31,21 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 79,160 }, { 44,136 }, { 32,21 }, 0, 0, false, false, 0 }, { { 0,2,5 }, { 137,160 }, { 34,90 }, { 31,21 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 0,43 }, { 104,498 }, { 18,34 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 18,41 }, { 1,258 }, { 42,36 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 60,41 }, { 1,146 }, { 41,36 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 129,44 }, { 1,432 }, { 43,33 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 52,14 }, { 101,328 }, { 26,28 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 28,14 }, { 89,212 }, { 26,28 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 4,14 }, { 89,182 }, { 26,28 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 44,2 }, { 117,186 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 44,2 }, { 100,88 }, { 7,12 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 44,2 }, { 117,172 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 44,2 }, { 14,418 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 44,2 }, { 117,228 }, { 10,12 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 44,2 }, { 25,418 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 44,2 }, { 36,418 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 44,2 }, { 1,244 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 44,2 }, { 12,244 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 44,2 }, { 23,244 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 33,2 }, { 34,244 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 33,2 }, { 102,102 }, { 8,12 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 33,2 }, { 112,158 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 33,2 }, { 112,144 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 33,2 }, { 117,214 }, { 10,12 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 33,2 }, { 112,130 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 33,2 }, { 104,484 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 33,2 }, { 115,484 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 33,2 }, { 104,470 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 33,2 }, { 115,470 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 22,2 }, { 104,456 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 22,2 }, { 112,102 }, { 8,12 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 22,2 }, { 115,456 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 22,2 }, { 104,442 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 22,2 }, { 1,418 }, { 11,12 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 22,2 }, { 115,442 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 22,2 }, { 104,428 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 22,2 }, { 115,428 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 22,2 }, { 101,314 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 22,2 }, { 101,300 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 12,2 }, { 112,314 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 12,2 }, { 109,88 }, { 7,12 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 12,2 }, { 112,300 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 12,2 }, { 102,379 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 12,2 }, { 117,200 }, { 10,12 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 12,2 }, { 113,379 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 12,2 }, { 102,365 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 12,2 }, { 113,365 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 12,2 }, { 102,116 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 12,2 }, { 113,116 }, { 9,12 }, 0, 0, false, false, 0 }, { { 0,0,10 }, { 64,97 }, { 48,182 }, { 39,58 }, 0, 1, false, false, 0 }, { { 0,0,11 }, { 122,98 }, { 1,357 }, { 49,57 }, 0, 1, false, false, 0 }, { { 0,0,6 }, { 4,18 }, { 1,296 }, { 43,59 }, 0, 1, false, false, 0 }, { { 0,0,7 }, { 60,17 }, { 68,534 }, { 51,60 }, 0, 1, false, false, 0 }, { { 0,0,8 }, { 125,14 }, { 1,467 }, { 47,65 }, 0, 1, false, false, 0 }, { { 0,0,9 }, { 2,98 }, { 1,184 }, { 45,56 }, 0, 1, false, false, 0 }, { { 0,1,10 }, { 48,90 }, { 1,595 }, { 65,59 }, 0, 1, false, false, 0 }, { { 0,1,11 }, { 107,90 }, { 1,534 }, { 65,59 }, 0, 1, false, false, 0 }, { { 0,1,6 }, { 0,14 }, { 73,952 }, { 54,71 }, 0, 1, false, false, 0 }, { { 0,1,7 }, { 47,14 }, { 1,950 }, { 70,73 }, 0, 1, false, false, 0 }, { { 0,1,8 }, { 116,8 }, { 63,721 }, { 56,77 }, 0, 1, false, false, 0 }, { { 0,1,9 }, { 0,90 }, { 1,656 }, { 61,63 }, 0, 1, false, false, 0 }, { { 0,2,10 }, { 75,83 }, { 1,123 }, { 32,21 }, 0, 1, false, false, 0 }, { { 0,2,11 }, { 137,83 }, { 1,77 }, { 31,21 }, 0, 1, false, false, 0 }, { { 0,2,6 }, { 14,1 }, { 78,136 }, { 32,21 }, 0, 1, false, false, 0 }, { { 0,2,7 }, { 73,1 }, { 35,113 }, { 32,21 }, 0, 1, false, false, 0 }, { { 0,2,8 }, { 147,1 }, { 102,393 }, { 25,21 }, 0, 1, false, false, 0 }, { { 0,2,9 }, { 0,83 }, { 67,90 }, { 31,21 }, 0, 1, false, false, 0 }
};  const size_t size_segment_d_and_j_climber = sizeof(segment_d_and_j_climber)/sizeof(segment_d_and_j_climber[0]); 
 const uint16_t segment_info_d_and_j_climber[] = { 128, 1024, 1, 0, 172, 240, 172, 159}; 

const std::string path_background_d_and_j_climber = "";
const uint16_t background_info_d_and_j_climber[] = { 256, 512, 0, 0, 172, 240, 0, 0, 172, 159, 1, 0, 0 }; 



const std::string path_console_d_and_j_climber = "romfs:/gfx/console_d_and_j_climber.t3x";
const uint16_t console_info_d_and_j_climber[] = { 512, 256, 0, 16, 320, 240}; 




const GW_rom d_and_j_climber (
    "David the climber", "D_AND_J_CLIMBER", "2024-08-26"
    , rom_d_and_j_climber, size_rom_d_and_j_climber
    , melody_d_and_j_climber, size_melody_d_and_j_climber
    , path_segment_d_and_j_climber
    , segment_d_and_j_climber, size_segment_d_and_j_climber
    , segment_info_d_and_j_climber
    , path_background_d_and_j_climber
    , background_info_d_and_j_climber
    , path_console_d_and_j_climber
    , console_info_d_and_j_climber
    , GW_rom::MANUFACTURER_DAVID_AND_JOHN
);

