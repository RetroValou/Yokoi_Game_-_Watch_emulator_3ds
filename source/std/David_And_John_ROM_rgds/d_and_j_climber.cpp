
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "d_and_j_climber.h"


const uint8_t rom_GW_d_and_j_climber[] = {
    0xFF, 0xFF, 0xFF, 0x01
}; const size_t size_rom_GW_d_and_j_climber = sizeof(rom_GW_d_and_j_climber)/sizeof(rom_GW_d_and_j_climber[0]);

const uint8_t melody_GW_d_and_j_climber[1] = {0}; 
	const size_t size_melody_GW_d_and_j_climber = 0;

const std::string path_segment_d_and_j_climber = "gfx/segment_d_and_j_climber.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_d_and_j_climber[] = {
	{ { 0,0,0 }, { 13,185 }, { 1,1088 }, { 101,123 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 126,193 }, { 91,512 }, { 90,113 }, 0, 0, false, false, 0 }, { { 0,0,12 }, { 199,42 }, { 183,512 }, { 61,113 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 239,193 }, { 134,1344 }, { 104,113 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 0,355 }, { 101,627 }, { 95,112 }, 0, 0, false, false, 0 }, { { 0,0,4 }, { 129,355 }, { 104,977 }, { 104,118 }, 0, 0, false, false, 0 }, { { 0,0,5 }, { 236,355 }, { 101,741 }, { 96,112 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 0,169 }, { 133,1611 }, { 119,147 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 115,174 }, { 114,1459 }, { 115,150 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 228,201 }, { 137,1780 }, { 117,121 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 0,358 }, { 125,1097 }, { 109,122 }, 0, 0, false, false, 0 }, { { 0,1,4 }, { 107,339 }, { 1,1760 }, { 134,141 }, 0, 0, false, false, 0 }, { { 0,1,5 }, { 215,339 }, { 1,1617 }, { 130,141 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 48,156 }, { 1,266 }, { 63,42 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 131,156 }, { 1,354 }, { 64,42 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 266,156 }, { 1,914 }, { 64,42 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 23,320 }, { 1,870 }, { 64,42 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 158,320 }, { 67,348 }, { 64,42 }, 0, 0, false, false, 0 }, { { 0,2,5 }, { 274,320 }, { 1,310 }, { 64,42 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 0,85 }, { 210,1025 }, { 36,70 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 37,83 }, { 172,364 }, { 82,72 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 120,83 }, { 172,438 }, { 83,72 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 258,88 }, { 1,550 }, { 87,67 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 104,29 }, { 201,920 }, { 53,55 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 56,29 }, { 201,863 }, { 53,55 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 7,29 }, { 201,806 }, { 53,55 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 88,4 }, { 236,1195 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 88,5 }, { 240,1434 }, { 15,23 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 88,4 }, { 236,1169 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 88,4 }, { 236,1143 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 88,5 }, { 231,1586 }, { 21,23 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 88,4 }, { 236,1117 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 88,4 }, { 104,1187 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 88,4 }, { 104,1161 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 88,4 }, { 104,1135 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 88,4 }, { 104,1109 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 66,4 }, { 133,366 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 66,5 }, { 67,855 }, { 16,23 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 66,4 }, { 231,1485 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 66,4 }, { 231,1459 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 66,5 }, { 231,1561 }, { 21,23 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 67,4 }, { 238,710 }, { 17,24 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 67,4 }, { 153,366 }, { 17,24 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 67,4 }, { 218,684 }, { 17,24 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 66,4 }, { 67,932 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 66,4 }, { 67,906 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 45,4 }, { 67,880 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 45,5 }, { 240,1409 }, { 15,23 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 45,4 }, { 1,524 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 45,4 }, { 21,524 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 45,5 }, { 231,1536 }, { 21,23 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 45,4 }, { 41,524 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 45,4 }, { 61,524 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 45,4 }, { 210,999 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 45,4 }, { 230,999 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 45,4 }, { 199,736 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 23,4 }, { 219,736 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 23,5 }, { 239,737 }, { 16,23 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 23,4 }, { 199,338 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 23,4 }, { 219,338 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 23,5 }, { 231,1511 }, { 21,23 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 23,4 }, { 199,312 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 23,4 }, { 219,312 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 23,4 }, { 198,710 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 23,4 }, { 218,710 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 23,4 }, { 198,684 }, { 18,24 }, 0, 0, false, false, 0 }, { { 0,0,10 }, { 128,193 }, { 93,392 }, { 77,118 }, 0, 1, false, false, 0 }, { { 0,0,11 }, { 244,196 }, { 1,738 }, { 98,115 }, 0, 1, false, false, 0 }, { { 0,0,6 }, { 7,37 }, { 1,619 }, { 88,117 }, 0, 1, false, false, 0 }, { { 0,0,7 }, { 120,34 }, { 98,855 }, { 101,120 }, 0, 1, false, false, 0 }, { { 0,0,8 }, { 249,29 }, { 1,958 }, { 95,128 }, 0, 1, false, false, 0 }, { { 0,0,9 }, { 5,196 }, { 1,398 }, { 90,112 }, 0, 1, false, false, 0 }, { { 0,1,10 }, { 96,180 }, { 1,1340 }, { 131,117 }, 0, 1, false, false, 0 }, { { 0,1,11 }, { 214,180 }, { 125,1221 }, { 130,117 }, 0, 1, false, false, 0 }, { { 0,1,6 }, { 0,29 }, { 145,1905 }, { 108,142 }, 0, 1, false, false, 0 }, { { 0,1,7 }, { 93,29 }, { 1,1903 }, { 142,144 }, 0, 1, false, false, 0 }, { { 0,1,8 }, { 233,15 }, { 1,1459 }, { 111,156 }, 0, 1, false, false, 0 }, { { 0,1,9 }, { 0,180 }, { 1,1213 }, { 122,125 }, 0, 1, false, false, 0 }, { { 0,2,10 }, { 150,166 }, { 133,276 }, { 63,42 }, 0, 1, false, false, 0 }, { { 0,2,11 }, { 273,166 }, { 133,320 }, { 64,42 }, 0, 1, false, false, 0 }, { { 0,2,6 }, { 29,2 }, { 66,260 }, { 63,42 }, 0, 1, false, false, 0 }, { { 0,2,7 }, { 147,2 }, { 67,304 }, { 64,42 }, 0, 1, false, false, 0 }, { { 0,2,8 }, { 295,2 }, { 199,762 }, { 49,42 }, 0, 1, false, false, 0 }, { { 0,2,9 }, { 0,166 }, { 1,222 }, { 63,42 }, 0, 1, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_climber = sizeof(segment_GW_d_and_j_climber)/sizeof(segment_GW_d_and_j_climber[0]); 
 const uint16_t segment_info_d_and_j_climber[] = { 256, 2048, 1, 0, 345, 480, 344, 318}; 

const std::string path_background_d_and_j_climber = "";
const uint16_t background_info_d_and_j_climber[] = { 512, 1024, 0, 0, 345, 480, 0, 0, 344, 318, 1, 0, 0 }; 


const std::string path_console_d_and_j_climber = "gfx/console_d_and_j_climber.png";
const uint16_t console_info_d_and_j_climber[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom d_and_j_climber (
    "David the climber", "D_AND_J_CLIMBER", "2024-08-26"
    , rom_GW_d_and_j_climber, size_rom_GW_d_and_j_climber
    , melody_GW_d_and_j_climber, size_melody_GW_d_and_j_climber
    , path_segment_d_and_j_climber
    , segment_GW_d_and_j_climber, size_segment_GW_d_and_j_climber
    , segment_info_d_and_j_climber
    , path_background_d_and_j_climber
    , background_info_d_and_j_climber
    , path_console_d_and_j_climber
    , console_info_d_and_j_climber
    , GW_rom::MANUFACTURER_DAVID_AND_JOHN
);

