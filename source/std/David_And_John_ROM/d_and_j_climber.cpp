
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

const std::string path_segment_d_and_j_climber = "romfs:/gfx/segment_d_and_j_climber.t3x"; // Visual of segment -> Big unique texture
const Segment segment_GW_d_and_j_climber[] = {
	{ { 0,0,0 }, { 8,111 }, { 64,699 }, { 60,74 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 76,116 }, { 1,617 }, { 54,68 }, 0, 0, false, false, 0 }, { { 0,0,12 }, { 119,25 }, { 212,790 }, { 37,68 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 143,116 }, { 126,639 }, { 63,68 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 1,214 }, { 1,457 }, { 43,26 }, 0, 0, false, false, 0 }, { { 0,0,4 }, { 77,214 }, { 163,511 }, { 63,26 }, 0, 0, false, false, 0 }, { { 0,0,5 }, { 148,214 }, { 47,466 }, { 51,26 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 0,102 }, { 88,935 }, { 72,88 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 69,105 }, { 162,933 }, { 69,90 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 137,121 }, { 137,709 }, { 70,72 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 0,215 }, { 105,485 }, { 65,24 }, 0, 0, false, false, 0 }, { { 0,1,4 }, { 71,204 }, { 172,490 }, { 73,19 }, 0, 0, false, false, 0 }, { { 0,1,5 }, { 148,204 }, { 105,514 }, { 16,9 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 29,94 }, { 87,422 }, { 38,25 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 79,94 }, { 42,411 }, { 38,25 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 160,94 }, { 1,403 }, { 38,25 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 14,193 }, { 174,427 }, { 38,25 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 95,193 }, { 214,427 }, { 38,25 }, 0, 0, false, false, 0 }, { { 0,2,5 }, { 164,192 }, { 46,438 }, { 39,26 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 0,51 }, { 233,981 }, { 22,42 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 22,50 }, { 60,581 }, { 50,43 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 72,50 }, { 60,536 }, { 50,43 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 155,53 }, { 51,494 }, { 52,40 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 63,17 }, { 100,449 }, { 31,34 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 33,17 }, { 172,454 }, { 32,34 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 4,17 }, { 206,454 }, { 32,34 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 53,3 }, { 87,450 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 53,3 }, { 246,965 }, { 9,14 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 53,3 }, { 239,543 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 53,3 }, { 239,527 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 53,3 }, { 240,474 }, { 12,14 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 53,3 }, { 228,511 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 53,3 }, { 241,511 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 53,3 }, { 233,965 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 53,3 }, { 233,949 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 53,3 }, { 233,933 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 40,3 }, { 230,917 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 40,3 }, { 246,949 }, { 9,14 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 40,3 }, { 243,917 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 40,3 }, { 230,901 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 40,3 }, { 240,458 }, { 12,14 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 40,3 }, { 243,901 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 40,3 }, { 230,885 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 40,3 }, { 243,885 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 40,3 }, { 230,869 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 40,3 }, { 243,869 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 27,3 }, { 1,387 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 27,3 }, { 246,933 }, { 9,14 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 27,3 }, { 14,387 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 27,3 }, { 27,387 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 27,3 }, { 239,575 }, { 12,14 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 27,3 }, { 1,371 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 27,3 }, { 14,371 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 27,3 }, { 27,371 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 27,3 }, { 167,411 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 27,3 }, { 180,411 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 14,3 }, { 193,411 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 14,3 }, { 126,759 }, { 9,14 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 14,3 }, { 206,411 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 14,3 }, { 219,411 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 14,3 }, { 239,559 }, { 12,14 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 14,3 }, { 232,411 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 14,3 }, { 167,395 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 14,3 }, { 122,384 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 14,3 }, { 135,384 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 14,3 }, { 148,384 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,0,10 }, { 77,165 }, { 209,710 }, { 46,71 }, 0, 1, false, false, 0 }, { { 0,0,11 }, { 147,167 }, { 191,638 }, { 59,69 }, 0, 1, false, false, 0 }, { { 0,0,3 }, { 0,0 }, { 1,574 }, { 57,41 }, 0, 1, false, false, 0 }, { { 0,0,4 }, { 79,0 }, { 1,528 }, { 48,44 }, 0, 1, false, false, 0 }, { { 0,0,5 }, { 142,0 }, { 1,485 }, { 44,41 }, 0, 1, false, false, 0 }, { { 0,0,6 }, { 4,71 }, { 60,626 }, { 53,71 }, 0, 1, false, false, 0 }, { { 0,0,7 }, { 72,69 }, { 1,766 }, { 61,73 }, 0, 1, false, false, 0 }, { { 0,0,8 }, { 150,66 }, { 1,687 }, { 57,77 }, 0, 1, false, false, 0 }, { { 0,0,9 }, { 3,167 }, { 115,570 }, { 54,67 }, 0, 1, false, false, 0 }, { { 0,1,10 }, { 58,157 }, { 70,862 }, { 78,71 }, 0, 1, false, false, 0 }, { { 0,1,11 }, { 129,157 }, { 150,860 }, { 78,71 }, 0, 1, false, false, 0 }, { { 0,1,3 }, { 0,8 }, { 112,525 }, { 49,43 }, 0, 1, false, false, 0 }, { { 0,1,4 }, { 64,1 }, { 171,539 }, { 66,50 }, 0, 1, false, false, 0 }, { { 0,1,5 }, { 129,6 }, { 171,591 }, { 78,45 }, 0, 1, false, false, 0 }, { { 0,1,6 }, { 0,66 }, { 70,775 }, { 65,85 }, 0, 1, false, false, 0 }, { { 0,1,7 }, { 56,66 }, { 1,936 }, { 85,87 }, 0, 1, false, false, 0 }, { { 0,1,8 }, { 140,58 }, { 1,841 }, { 67,93 }, 0, 1, false, false, 0 }, { { 0,1,9 }, { 0,157 }, { 137,783 }, { 73,75 }, 0, 1, false, false, 0 }, { { 0,2,10 }, { 90,149 }, { 127,400 }, { 38,25 }, 0, 1, false, false, 0 }, { { 0,2,11 }, { 164,149 }, { 1,430 }, { 39,25 }, 0, 1, false, false, 0 }, { { 0,2,6 }, { 17,50 }, { 82,395 }, { 38,25 }, 0, 1, false, false, 0 }, { { 0,2,7 }, { 88,50 }, { 133,427 }, { 39,25 }, 0, 1, false, false, 0 }, { { 0,2,8 }, { 177,50 }, { 133,458 }, { 30,25 }, 0, 1, false, false, 0 }, { { 0,2,9 }, { 0,149 }, { 41,384 }, { 38,25 }, 0, 1, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_climber = sizeof(segment_GW_d_and_j_climber)/sizeof(segment_GW_d_and_j_climber[0]); 
 const uint16_t segment_info_d_and_j_climber[] = { 256, 1024, 1, 0, 207, 240, 207, 240}; 

const std::string path_background_d_and_j_climber = "";
const uint16_t background_info_d_and_j_climber[] = { 256, 512, 0, 0, 207, 240, 0, 0, 207, 240, 1, 0, 0 }; 


const std::string path_console_d_and_j_climber = "romfs:/gfx/console_d_and_j_climber.t3x";
const uint16_t console_info_d_and_j_climber[] = { 512, 256, 0, 16, 320, 240}; 




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

