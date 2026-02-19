
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
	{ { 0,0,0 }, { 15,223 }, { 1,1390 }, { 122,148 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 151,232 }, { 1,1112 }, { 109,136 }, 0, 0, false, false, 0 }, { { 0,0,12 }, { 239,51 }, { 429,1587 }, { 73,135 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 288,232 }, { 249,1275 }, { 125,136 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 2,427 }, { 420,1430 }, { 87,53 }, 0, 0, false, false, 0 }, { { 0,0,4 }, { 155,427 }, { 210,1021 }, { 125,53 }, 0, 0, false, false, 0 }, { { 0,0,5 }, { 297,427 }, { 284,961 }, { 103,53 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 0,203 }, { 174,1870 }, { 144,177 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 138,210 }, { 320,1867 }, { 139,180 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 274,242 }, { 160,1422 }, { 141,145 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 0,430 }, { 1,973 }, { 131,48 }, 0, 0, false, false, 0 }, { { 0,1,4 }, { 142,408 }, { 134,982 }, { 148,37 }, 0, 0, false, false, 0 }, { { 0,1,5 }, { 297,408 }, { 125,1427 }, { 32,18 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 57,187 }, { 265,908 }, { 77,51 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 158,187 }, { 1,920 }, { 77,51 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 320,187 }, { 1,867 }, { 77,51 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 28,385 }, { 344,894 }, { 77,51 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 190,385 }, { 423,894 }, { 77,51 }, 0, 0, false, false, 0 }, { { 0,2,5 }, { 330,385 }, { 1,814 }, { 76,51 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 0,103 }, { 461,1964 }, { 43,83 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 44,100 }, { 393,1098 }, { 100,86 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 145,99 }, { 160,1608 }, { 99,87 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 310,106 }, { 367,1016 }, { 105,80 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 125,35 }, { 389,947 }, { 64,67 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 67,35 }, { 134,913 }, { 64,67 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 9,35 }, { 200,913 }, { 63,67 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 106,5 }, { 315,846 }, { 21,29 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 106,5 }, { 242,824 }, { 18,29 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 106,5 }, { 210,1107 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 106,5 }, { 210,1076 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 106,5 }, { 484,1400 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 106,5 }, { 367,1147 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 106,5 }, { 367,1116 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 106,5 }, { 485,1903 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 106,5 }, { 485,1873 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 106,5 }, { 219,824 }, { 21,29 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 80,5 }, { 484,1279 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 80,5 }, { 262,824 }, { 18,29 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 80,5 }, { 315,877 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 80,5 }, { 337,1045 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 80,5 }, { 484,1370 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 80,5 }, { 160,1577 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 80,5 }, { 184,1577 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 80,5 }, { 461,1872 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 80,5 }, { 80,943 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 80,5 }, { 208,1577 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 54,5 }, { 232,1577 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 54,5 }, { 282,824 }, { 18,29 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 54,5 }, { 125,1509 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 54,5 }, { 125,1478 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 54,5 }, { 484,1340 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 54,5 }, { 125,1447 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 54,5 }, { 474,1067 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 54,5 }, { 80,913 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 54,5 }, { 104,943 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 54,5 }, { 474,1036 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 28,5 }, { 474,1005 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 28,5 }, { 363,863 }, { 18,29 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 28,5 }, { 455,974 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 28,5 }, { 479,974 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 28,5 }, { 484,1310 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 28,5 }, { 461,1933 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 28,5 }, { 485,1933 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 28,5 }, { 104,913 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 28,5 }, { 339,864 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 28,5 }, { 461,1902 }, { 22,29 }, 0, 0, false, false, 0 }, { { 0,0,10 }, { 155,330 }, { 419,1724 }, { 92,141 }, 0, 1, false, false, 0 }, { { 0,0,11 }, { 294,333 }, { 1,1250 }, { 119,138 }, 0, 1, false, false, 0 }, { { 0,0,3 }, { 0,0 }, { 393,1186 }, { 115,82 }, 0, 1, false, false, 0 }, { { 0,0,4 }, { 158,0 }, { 112,1047 }, { 96,89 }, 0, 1, false, false, 0 }, { { 0,0,5 }, { 284,0 }, { 420,1485 }, { 90,82 }, 0, 1, false, false, 0 }, { { 0,0,6 }, { 9,142 }, { 376,1270 }, { 106,141 }, 0, 1, false, false, 0 }, { { 0,0,7 }, { 145,138 }, { 125,1275 }, { 122,145 }, 0, 1, false, false, 0 }, { { 0,0,8 }, { 300,132 }, { 303,1413 }, { 115,154 }, 0, 1, false, false, 0 }, { { 0,0,9 }, { 5,333 }, { 122,1138 }, { 110,135 }, 0, 1, false, false, 0 }, { { 0,1,10 }, { 116,313 }, { 270,1569 }, { 157,142 }, 0, 1, false, false, 0 }, { { 0,1,11 }, { 258,313 }, { 1,1540 }, { 157,142 }, 0, 1, false, false, 0 }, { { 0,1,3 }, { 0,15 }, { 1,1023 }, { 98,87 }, 0, 1, false, false, 0 }, { { 0,1,4 }, { 129,2 }, { 234,1076 }, { 131,100 }, 0, 1, false, false, 0 }, { { 0,1,5 }, { 258,12 }, { 234,1178 }, { 157,90 }, 0, 1, false, false, 0 }, { { 0,1,6 }, { 0,132 }, { 137,1697 }, { 131,171 }, 0, 1, false, false, 0 }, { { 0,1,7 }, { 112,132 }, { 1,1873 }, { 171,174 }, 0, 1, false, false, 0 }, { { 0,1,8 }, { 281,116 }, { 1,1684 }, { 134,187 }, 0, 1, false, false, 0 }, { { 0,1,9 }, { 0,313 }, { 270,1713 }, { 147,152 }, 0, 1, false, false, 0 }, { { 0,2,10 }, { 180,297 }, { 80,860 }, { 77,51 }, 0, 1, false, false, 0 }, { { 0,2,11 }, { 330,297 }, { 159,860 }, { 76,51 }, 0, 1, false, false, 0 }, { { 0,2,6 }, { 35,100 }, { 1,762 }, { 76,50 }, 0, 1, false, false, 0 }, { { 0,2,7 }, { 177,100 }, { 79,808 }, { 77,50 }, 0, 1, false, false, 0 }, { { 0,2,8 }, { 356,100 }, { 158,808 }, { 59,50 }, 0, 1, false, false, 0 }, { { 0,2,9 }, { 0,297 }, { 237,855 }, { 76,51 }, 0, 1, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_climber = sizeof(segment_GW_d_and_j_climber)/sizeof(segment_GW_d_and_j_climber[0]); 
 const uint16_t segment_info_d_and_j_climber[] = { 512, 2048, 1, 0, 415, 480, 415, 480}; 

const std::string path_background_d_and_j_climber = "";
const uint16_t background_info_d_and_j_climber[] = { 512, 1024, 0, 0, 415, 480, 0, 0, 415, 480, 1, 0, 0 }; 


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

