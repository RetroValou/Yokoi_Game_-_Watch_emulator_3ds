
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "d_and_j_rythme.h"


const uint8_t rom_GW_d_and_j_rythme[] = {
    0xFF, 0xFF, 0xFF, 0x03
}; const size_t size_rom_GW_d_and_j_rythme = sizeof(rom_GW_d_and_j_rythme)/sizeof(rom_GW_d_and_j_rythme[0]);

const uint8_t melody_GW_d_and_j_rythme[1] = {0}; 
	const size_t size_melody_GW_d_and_j_rythme = 0;

const std::string path_segment_d_and_j_rythme = "gfx/segment_d_and_j_rythme.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_d_and_j_rythme[] = {
	{ { 0,12,0 }, { 74,197 }, { 405,457 }, { 41,54 }, 0, 0, false, false, 0 }, { { 0,12,1 }, { 216,197 }, { 448,457 }, { 41,54 }, 0, 0, false, false, 0 }, { { 0,12,10 }, { 158,340 }, { 298,336 }, { 41,54 }, 0, 0, false, false, 0 }, { { 0,12,11 }, { 300,340 }, { 361,457 }, { 42,54 }, 0, 0, false, false, 0 }, { { 0,12,2 }, { 359,197 }, { 298,280 }, { 41,54 }, 0, 0, false, false, 0 }, { { 0,12,3 }, { 15,197 }, { 341,336 }, { 41,54 }, 0, 0, false, false, 0 }, { { 0,12,4 }, { 158,197 }, { 377,401 }, { 41,54 }, 0, 0, false, false, 0 }, { { 0,12,5 }, { 300,197 }, { 338,64 }, { 42,54 }, 0, 0, false, false, 0 }, { { 0,12,6 }, { 74,340 }, { 384,290 }, { 41,53 }, 0, 0, false, false, 0 }, { { 0,12,7 }, { 216,340 }, { 400,235 }, { 41,53 }, 0, 0, false, false, 0 }, { { 0,12,8 }, { 359,340 }, { 427,290 }, { 41,53 }, 0, 0, false, false, 0 }, { { 0,12,9 }, { 15,340 }, { 341,280 }, { 41,54 }, 0, 0, false, false, 0 }, { { 0,14,0 }, { 0,129 }, { 102,20 }, { 131,57 }, 0, 0, false, false, 0 }, { { 0,14,1 }, { 142,129 }, { 106,297 }, { 131,57 }, 0, 0, false, false, 0 }, { { 0,14,2 }, { 284,129 }, { 106,238 }, { 131,57 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 135,200 }, { 443,8 }, { 6,280 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 278,200 }, { 506,231 }, { 5,280 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 125,51 }, { 235,45 }, { 64,67 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 67,51 }, { 295,444 }, { 64,67 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 9,51 }, { 292,196 }, { 63,67 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 106,12 }, { 315,14 }, { 21,28 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 106,12 }, { 154,208 }, { 18,28 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 106,12 }, { 384,371 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 106,12 }, { 174,84 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 106,12 }, { 116,359 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 106,12 }, { 198,84 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 106,12 }, { 106,208 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 106,12 }, { 130,208 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 106,12 }, { 267,15 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 106,12 }, { 475,161 }, { 21,28 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 80,12 }, { 408,205 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 80,12 }, { 451,131 }, { 18,28 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 80,12 }, { 408,175 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 80,12 }, { 408,145 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 80,12 }, { 143,359 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 80,12 }, { 382,138 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 80,12 }, { 470,371 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 80,12 }, { 470,341 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 80,12 }, { 470,311 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 80,12 }, { 470,281 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 54,12 }, { 406,115 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 54,12 }, { 471,131 }, { 18,28 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 54,12 }, { 382,108 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 54,12 }, { 406,85 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 54,12 }, { 170,359 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 54,12 }, { 382,78 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 54,12 }, { 406,55 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 54,12 }, { 382,48 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 54,12 }, { 406,25 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 54,12 }, { 382,18 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 28,12 }, { 291,14 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 28,12 }, { 491,131 }, { 18,28 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 28,12 }, { 451,251 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 28,12 }, { 475,251 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 28,12 }, { 197,359 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 28,12 }, { 451,221 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 28,12 }, { 475,221 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 28,12 }, { 451,191 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 28,12 }, { 475,191 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 28,12 }, { 451,161 }, { 22,28 }, 0, 0, false, false, 0 }, { { 0,0,0 }, { 5,216 }, { 1,356 }, { 113,155 }, 0, 1, false, false, 0 }, { { 0,0,1 }, { 148,213 }, { 1,39 }, { 99,158 }, 0, 1, false, false, 0 }, { { 0,0,2 }, { 297,216 }, { 1,199 }, { 103,155 }, 0, 1, false, false, 0 }, { { 0,1,0 }, { 2,294 }, { 292,120 }, { 35,74 }, 0, 1, false, false, 0 }, { { 0,1,1 }, { 145,294 }, { 329,120 }, { 34,74 }, 0, 1, false, false, 0 }, { { 0,1,2 }, { 368,294 }, { 301,44 }, { 35,74 }, 0, 1, false, false, 0 }, { { 0,10,0 }, { 142,421 }, { 295,392 }, { 80,50 }, 0, 1, false, false, 0 }, { { 0,11,0 }, { 122,430 }, { 400,227 }, { 6,6 }, 0, 1, false, false, 0 }, { { 0,11,1 }, { 122,417 }, { 384,350 }, { 22,19 }, 0, 1, false, false, 0 }, { { 0,11,2 }, { 122,411 }, { 239,238 }, { 22,25 }, 0, 1, false, false, 0 }, { { 0,12,12 }, { 74,2 }, { 357,224 }, { 41,54 }, 0, 1, false, false, 0 }, { { 0,12,13 }, { 216,2 }, { 365,168 }, { 41,54 }, 0, 1, false, false, 0 }, { { 0,12,14 }, { 359,2 }, { 420,401 }, { 41,54 }, 0, 1, false, false, 0 }, { { 0,12,15 }, { 15,2 }, { 463,401 }, { 41,54 }, 0, 1, false, false, 0 }, { { 0,12,16 }, { 158,2 }, { 408,345 }, { 41,54 }, 0, 1, false, false, 0 }, { { 0,12,17 }, { 300,2 }, { 338,8 }, { 42,54 }, 0, 1, false, false, 0 }, { { 0,13,0 }, { 67,64 }, { 236,389 }, { 57,122 }, 0, 1, false, false, 0 }, { { 0,13,1 }, { 210,64 }, { 174,114 }, { 57,122 }, 0, 1, false, false, 0 }, { { 0,13,2 }, { 352,64 }, { 116,389 }, { 58,122 }, 0, 1, false, false, 0 }, { { 0,13,3 }, { 5,64 }, { 176,389 }, { 58,122 }, 0, 1, false, false, 0 }, { { 0,13,4 }, { 148,64 }, { 233,114 }, { 57,122 }, 0, 1, false, false, 0 }, { { 0,13,5 }, { 291,64 }, { 239,265 }, { 57,122 }, 0, 1, false, false, 0 }, { { 0,2,0 }, { 135,0 }, { 365,120 }, { 6,46 }, 0, 1, false, false, 0 }, { { 0,2,1 }, { 278,0 }, { 499,353 }, { 5,46 }, 0, 1, false, false, 0 }, { { 0,3,0 }, { 0,122 }, { 1,12 }, { 131,6 }, 0, 1, false, false, 0 }, { { 0,3,1 }, { 142,122 }, { 1,4 }, { 131,6 }, 0, 1, false, false, 0 }, { { 0,3,2 }, { 284,122 }, { 134,12 }, { 131,6 }, 0, 1, false, false, 0 }, { { 0,4,0 }, { 229,362 }, { 102,79 }, { 70,118 }, 0, 1, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_rythme = sizeof(segment_GW_d_and_j_rythme)/sizeof(segment_GW_d_and_j_rythme[0]); 
 const uint16_t segment_info_d_and_j_rythme[] = { 512, 512, 1, 0, 415, 480, 415, 480}; 

const std::string path_background_d_and_j_rythme = "";
const uint16_t background_info_d_and_j_rythme[] = { 512, 1024, 0, 0, 415, 480, 0, 0, 415, 480, 1, 0, 0 }; 


const std::string path_console_d_and_j_rythme = "gfx/console_d_and_j_rythme.png";
const uint16_t console_info_d_and_j_rythme[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom d_and_j_rythme (
    "Please Stop sleeping David !", "D_AND_J_RYTHME", "2024-08-26"
    , rom_GW_d_and_j_rythme, size_rom_GW_d_and_j_rythme
    , melody_GW_d_and_j_rythme, size_melody_GW_d_and_j_rythme
    , path_segment_d_and_j_rythme
    , segment_GW_d_and_j_rythme, size_segment_GW_d_and_j_rythme
    , segment_info_d_and_j_rythme
    , path_background_d_and_j_rythme
    , background_info_d_and_j_rythme
    , path_console_d_and_j_rythme
    , console_info_d_and_j_rythme
    , GW_rom::MANUFACTURER_DAVID_AND_JOHN
);

