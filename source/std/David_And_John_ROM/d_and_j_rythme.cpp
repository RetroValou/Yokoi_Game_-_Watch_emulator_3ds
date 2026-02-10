
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

const std::string path_segment_d_and_j_rythme = "romfs:/gfx/segment_d_and_j_rythme.t3x"; // Visual of segment -> Big unique texture
const Segment segment_GW_d_and_j_rythme[] = {
	{ { 0,0,0 }, { 1,174 }, { 1,472 }, { 28,39 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 37,173 }, { 1,430 }, { 24,40 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 74,174 }, { 31,472 }, { 25,39 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 0,193 }, { 5,302 }, { 9,19 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 36,193 }, { 41,303 }, { 9,19 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 91,193 }, { 52,303 }, { 9,19 }, 0, 0, false, false, 0 }, { { 0,10,0 }, { 35,225 }, { 5,341 }, { 20,13 }, 0, 0, false, false, 0 }, { { 0,11,0 }, { 30,227 }, { 1,268 }, { 2,2 }, 0, 0, false, false, 0 }, { { 0,11,1 }, { 30,224 }, { 1,423 }, { 6,5 }, 0, 0, false, false, 0 }, { { 0,11,2 }, { 30,223 }, { 23,324 }, { 6,6 }, 0, 0, false, false, 0 }, { { 0,12,0 }, { 18,49 }, { 5,258 }, { 10,14 }, 0, 0, false, false, 0 }, { { 0,12,1 }, { 54,49 }, { 31,271 }, { 10,14 }, 0, 0, false, false, 0 }, { { 0,12,10 }, { 39,85 }, { 16,290 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,12,11 }, { 74,85 }, { 29,287 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,12,12 }, { 18,120 }, { 43,271 }, { 10,14 }, 0, 0, false, false, 0 }, { { 0,12,13 }, { 54,120 }, { 17,255 }, { 10,14 }, 0, 0, false, false, 0 }, { { 0,12,14 }, { 89,120 }, { 1,242 }, { 10,14 }, 0, 0, false, false, 0 }, { { 0,12,15 }, { 4,120 }, { 29,255 }, { 10,14 }, 0, 0, false, false, 0 }, { { 0,12,16 }, { 39,120 }, { 42,287 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,12,17 }, { 74,120 }, { 5,274 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,12,2 }, { 89,49 }, { 41,255 }, { 10,14 }, 0, 0, false, false, 0 }, { { 0,12,3 }, { 4,49 }, { 53,255 }, { 10,14 }, 0, 0, false, false, 0 }, { { 0,12,4 }, { 39,49 }, { 13,239 }, { 10,14 }, 0, 0, false, false, 0 }, { { 0,12,5 }, { 74,49 }, { 18,271 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,12,6 }, { 18,85 }, { 25,240 }, { 10,13 }, 0, 0, false, false, 0 }, { { 0,12,7 }, { 54,85 }, { 37,240 }, { 10,13 }, 0, 0, false, false, 0 }, { { 0,12,8 }, { 89,85 }, { 49,240 }, { 10,13 }, 0, 0, false, false, 0 }, { { 0,12,9 }, { 4,85 }, { 1,226 }, { 10,14 }, 0, 0, false, false, 0 }, { { 0,13,0 }, { 17,136 }, { 1,356 }, { 14,31 }, 0, 0, false, false, 0 }, { { 0,13,1 }, { 52,136 }, { 17,356 }, { 14,31 }, 0, 0, false, false, 0 }, { { 0,13,2 }, { 87,136 }, { 35,390 }, { 15,31 }, 0, 0, false, false, 0 }, { { 0,13,3 }, { 1,136 }, { 35,357 }, { 15,31 }, 0, 0, false, false, 0 }, { { 0,13,4 }, { 37,136 }, { 33,324 }, { 14,31 }, 0, 0, false, false, 0 }, { { 0,13,5 }, { 72,136 }, { 49,324 }, { 14,31 }, 0, 0, false, false, 0 }, { { 0,14,0 }, { 0,32 }, { 1,406 }, { 32,15 }, 0, 0, false, false, 0 }, { { 0,14,1 }, { 35,32 }, { 27,423 }, { 33,15 }, 0, 0, false, false, 0 }, { { 0,14,2 }, { 71,32 }, { 1,389 }, { 32,15 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 33,50 }, { 1,272 }, { 2,82 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 69,50 }, { 62,370 }, { 1,82 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 0,150 }, { 25,236 }, { 33,2 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 35,150 }, { 13,232 }, { 33,2 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 70,150 }, { 13,228 }, { 33,2 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 57,210 }, { 27,440 }, { 17,30 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 31,13 }, { 46,454 }, { 16,16 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 17,13 }, { 5,323 }, { 16,16 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 2,13 }, { 23,306 }, { 16,16 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 26,3 }, { 52,405 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 26,3 }, { 58,504 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 26,3 }, { 52,396 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 26,3 }, { 52,387 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 26,3 }, { 52,378 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 26,3 }, { 52,369 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 26,3 }, { 23,332 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 26,3 }, { 55,294 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 26,3 }, { 55,285 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 26,3 }, { 55,276 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 20,3 }, { 58,495 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 20,3 }, { 27,347 }, { 4,7 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 20,3 }, { 58,486 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 20,3 }, { 58,477 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 20,3 }, { 52,360 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 20,3 }, { 16,314 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 20,3 }, { 45,218 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 20,3 }, { 52,218 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 20,3 }, { 9,210 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 20,3 }, { 1,208 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 13,3 }, { 5,293 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 13,3 }, { 16,210 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 13,3 }, { 46,445 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 13,3 }, { 54,445 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 13,3 }, { 52,414 }, { 7,7 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 13,3 }, { 48,227 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 13,3 }, { 56,227 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 13,3 }, { 13,219 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 13,3 }, { 1,217 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 13,3 }, { 21,219 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 7,3 }, { 23,210 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 7,3 }, { 30,210 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 7,3 }, { 37,210 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 7,3 }, { 44,209 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 7,3 }, { 29,219 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 7,3 }, { 51,209 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 7,3 }, { 37,219 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 7,3 }, { 58,209 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 7,3 }, { 8,201 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 7,3 }, { 1,199 }, { 5,7 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_rythme = sizeof(segment_GW_d_and_j_rythme)/sizeof(segment_GW_d_and_j_rythme[0]); 
 const uint16_t segment_info_d_and_j_rythme[] = { 64, 512, 1, 0, 103, 240}; 

const std::string path_background_d_and_j_rythme = "";
const uint16_t background_info_d_and_j_rythme[] = { 128, 256, 0, 0, 103, 240, 1, 0, 0 }; 


const std::string path_console_d_and_j_rythme = "romfs:/gfx/console_d_and_j_rythme.t3x";
const uint16_t console_info_d_and_j_rythme[] = { 512, 256, 0, 16, 320, 240}; 




const GW_rom d_and_j_rythme (
    "Stop sleeping David !", "D_AND_J_RYTHME", "2024-08-26"
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

