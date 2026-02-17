
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

const std::string path_segment_d_and_j_esquive = "gfx/segment_d_and_j_esquive.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_d_and_j_esquive[] = {
	{ { 0,0,0 }, { 340,17 }, { 57,506 }, { 64,48 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 346,96 }, { 69,746 }, { 58,64 }, 0, 0, false, false, 0 }, { { 0,0,10 }, { 6,92 }, { 1,748 }, { 62,64 }, 0, 0, false, false, 0 }, { { 0,0,11 }, { 12,174 }, { 69,680 }, { 56,64 }, 0, 0, false, false, 0 }, { { 0,0,12 }, { 583,62 }, { 1,923 }, { 55,100 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 338,176 }, { 57,457 }, { 61,47 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 238,25 }, { 67,556 }, { 54,60 }, 0, 0, false, false, 0 }, { { 0,0,4 }, { 228,94 }, { 1,625 }, { 66,58 }, 0, 0, false, false, 0 }, { { 0,0,5 }, { 236,176 }, { 69,618 }, { 56,60 }, 0, 0, false, false, 0 }, { { 0,0,6 }, { 116,17 }, { 1,685 }, { 64,61 }, 0, 0, false, false, 0 }, { { 0,0,7 }, { 126,101 }, { 1,504 }, { 54,59 }, 0, 0, false, false, 0 }, { { 0,0,8 }, { 113,178 }, { 1,565 }, { 64,58 }, 0, 0, false, false, 0 }, { { 0,0,9 }, { 1,17 }, { 69,812 }, { 58,70 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 454,32 }, { 1,894 }, { 49,27 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 454,111 }, { 1,85 }, { 40,26 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 461,183 }, { 93,259 }, { 33,31 }, 0, 0, false, false, 0 }, { { 0,10,0 }, { 506,8 }, { 123,399 }, { 3,217 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 407,14 }, { 1,448 }, { 51,54 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 404,103 }, { 1,203 }, { 45,42 }, 0, 0, false, false, 0 }, { { 0,2,10 }, { 70,92 }, { 54,402 }, { 51,53 }, 0, 0, false, false, 0 }, { { 0,2,11 }, { 68,180 }, { 1,292 }, { 45,43 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 407,170 }, { 1,393 }, { 51,53 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 292,25 }, { 48,202 }, { 45,42 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 295,92 }, { 54,347 }, { 51,53 }, 0, 0, false, false, 0 }, { { 0,2,5 }, { 292,180 }, { 1,247 }, { 45,43 }, 0, 0, false, false, 0 }, { { 0,2,6 }, { 183,14 }, { 1,337 }, { 50,54 }, 0, 0, false, false, 0 }, { { 0,2,7 }, { 180,103 }, { 1,159 }, { 45,42 }, 0, 0, false, false, 0 }, { { 0,2,8 }, { 183,170 }, { 53,292 }, { 50,53 }, 0, 0, false, false, 0 }, { { 0,2,9 }, { 68,25 }, { 48,158 }, { 45,42 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 484,32 }, { 52,912 }, { 4,3 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 484,111 }, { 52,917 }, { 4,4 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 484,183 }, { 52,907 }, { 4,3 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 458,25 }, { 89,146 }, { 38,10 }, 0, 0, false, false, 0 }, { { 0,3,4 }, { 458,105 }, { 89,134 }, { 38,10 }, 0, 0, false, false, 0 }, { { 0,3,5 }, { 458,176 }, { 89,122 }, { 38,10 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 519,6 }, { 58,884 }, { 66,68 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 519,79 }, { 1,814 }, { 66,68 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 519,152 }, { 58,954 }, { 66,69 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 510,228 }, { 48,246 }, { 43,44 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 549,228 }, { 1,113 }, { 42,44 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 588,228 }, { 45,112 }, { 42,44 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 484,245 }, { 105,294 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 484,245 }, { 32,1 }, { 12,19 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 484,245 }, { 95,238 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 484,245 }, { 112,238 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 484,245 }, { 107,378 }, { 17,19 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 484,245 }, { 95,217 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 484,245 }, { 112,217 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 484,245 }, { 95,196 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 484,245 }, { 112,196 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 484,245 }, { 95,175 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 467,245 }, { 107,436 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 467,245 }, { 33,28 }, { 12,19 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 467,245 }, { 107,415 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 467,245 }, { 77,80 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 467,245 }, { 107,357 }, { 16,19 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 467,245 }, { 43,70 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 467,245 }, { 59,70 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 467,245 }, { 93,80 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 467,245 }, { 109,80 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 467,245 }, { 1,64 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 450,245 }, { 17,64 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 450,245 }, { 46,7 }, { 12,19 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 450,245 }, { 75,59 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 450,245 }, { 91,59 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 450,245 }, { 107,336 }, { 16,19 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 450,245 }, { 107,59 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 450,245 }, { 33,49 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 450,245 }, { 49,49 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 450,245 }, { 1,43 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 450,245 }, { 17,43 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 432,245 }, { 112,175 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 432,245 }, { 17,1 }, { 13,19 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 432,245 }, { 89,101 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 432,245 }, { 106,101 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 433,245 }, { 105,315 }, { 16,19 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 433,245 }, { 1,22 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 433,245 }, { 1,1 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 433,245 }, { 17,22 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 432,245 }, { 43,91 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 432,245 }, { 60,91 }, { 15,19 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_esquive = sizeof(segment_GW_d_and_j_esquive)/sizeof(segment_GW_d_and_j_esquive[0]); 
 const uint16_t segment_info_d_and_j_esquive[] = { 128, 1024, 1, 0, 638, 276}; 

const std::string path_background_d_and_j_esquive = "";
const uint16_t background_info_d_and_j_esquive[] = { 1024, 512, 0, 0, 638, 276, 1, 0, 0 }; 


const std::string path_console_d_and_j_esquive = "gfx/console_d_and_j_esquive.png";
const uint16_t console_info_d_and_j_esquive[] = { 1024, 512, 0, 32, 640, 480}; 




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

