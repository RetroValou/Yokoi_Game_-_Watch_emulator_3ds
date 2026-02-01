
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
const Segment segment_d_and_j_climber[] = {
	{ { 0,0,0 }, { 4,56 }, { 1,430 }, { 30,37 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 37,58 }, { 33,355 }, { 28,34 }, 0, 0, false, false, 0 }, { { 0,0,10 }, { 38,202 }, { 40,683 }, { 23,36 }, 0, 0, false, false, 0 }, { { 0,0,11 }, { 73,203 }, { 1,354 }, { 30,35 }, 0, 0, false, false, 0 }, { { 0,0,12 }, { 59,13 }, { 43,943 }, { 19,34 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 71,58 }, { 1,391 }, { 31,34 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 0,107 }, { 33,319 }, { 28,34 }, 0, 0, false, false, 0 }, { { 0,0,4 }, { 38,107 }, { 1,469 }, { 32,35 }, 0, 0, false, false, 0 }, { { 0,0,5 }, { 70,107 }, { 34,391 }, { 29,34 }, 0, 0, false, false, 0 }, { { 0,0,6 }, { 2,155 }, { 36,757 }, { 26,36 }, 0, 0, false, false, 0 }, { { 0,0,7 }, { 36,155 }, { 33,427 }, { 30,36 }, 0, 0, false, false, 0 }, { { 0,0,8 }, { 75,153 }, { 35,465 }, { 28,39 }, 0, 0, false, false, 0 }, { { 0,0,9 }, { 1,203 }, { 36,721 }, { 27,34 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 0,51 }, { 1,795 }, { 36,44 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 34,52 }, { 1,841 }, { 35,46 }, 0, 0, false, false, 0 }, { { 0,1,10 }, { 29,198 }, { 1,623 }, { 39,36 }, 0, 0, false, false, 0 }, { { 0,1,11 }, { 64,198 }, { 1,585 }, { 39,36 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 68,60 }, { 1,546 }, { 35,37 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 0,107 }, { 1,506 }, { 33,38 }, 0, 0, false, false, 0 }, { { 0,1,4 }, { 32,102 }, { 1,934 }, { 40,43 }, 0, 0, false, false, 0 }, { { 0,1,5 }, { 64,102 }, { 1,889 }, { 39,43 }, 0, 0, false, false, 0 }, { { 0,1,6 }, { 0,153 }, { 1,701 }, { 33,43 }, 0, 0, false, false, 0 }, { { 0,1,7 }, { 28,153 }, { 1,979 }, { 42,44 }, 0, 0, false, false, 0 }, { { 0,1,8 }, { 70,149 }, { 1,746 }, { 33,47 }, 0, 0, false, false, 0 }, { { 0,1,9 }, { 0,198 }, { 1,661 }, { 37,38 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 14,47 }, { 36,509 }, { 19,12 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 39,47 }, { 43,928 }, { 19,13 }, 0, 0, false, false, 0 }, { { 0,2,10 }, { 45,194 }, { 42,649 }, { 19,13 }, 0, 0, false, false, 0 }, { { 0,2,11 }, { 82,194 }, { 42,634 }, { 19,13 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 79,47 }, { 39,827 }, { 19,12 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 7,96 }, { 42,619 }, { 19,13 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 47,96 }, { 42,604 }, { 19,13 }, 0, 0, false, false, 0 }, { { 0,2,5 }, { 82,96 }, { 42,589 }, { 19,13 }, 0, 0, false, false, 0 }, { { 0,2,6 }, { 9,145 }, { 42,574 }, { 19,13 }, 0, 0, false, false, 0 }, { { 0,2,7 }, { 44,145 }, { 42,913 }, { 19,13 }, 0, 0, false, false, 0 }, { { 0,2,8 }, { 88,145 }, { 39,812 }, { 15,13 }, 0, 0, false, false, 0 }, { { 0,2,9 }, { 0,194 }, { 42,898 }, { 19,13 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 0,26 }, { 38,551 }, { 11,21 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 11,25 }, { 38,865 }, { 25,22 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 36,25 }, { 38,841 }, { 25,22 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 77,26 }, { 36,523 }, { 26,21 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 31,8 }, { 45,1006 }, { 16,17 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 17,8 }, { 45,987 }, { 16,17 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 2,8 }, { 40,664 }, { 16,17 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 26,1 }, { 57,513 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 26,1 }, { 58,664 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 26,1 }, { 56,817 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 26,1 }, { 56,807 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 26,1 }, { 51,889 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 26,1 }, { 39,797 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 26,1 }, { 47,802 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 26,1 }, { 9,335 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 26,1 }, { 17,335 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 26,1 }, { 55,797 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 20,1 }, { 58,673 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 20,1 }, { 59,564 }, { 4,8 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 20,1 }, { 25,325 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 20,1 }, { 9,316 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 20,1 }, { 25,335 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 20,1 }, { 1,315 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 20,1 }, { 16,316 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 20,1 }, { 15,297 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 20,1 }, { 1,296 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 20,1 }, { 23,315 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 13,1 }, { 51,564 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 13,1 }, { 37,310 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 13,1 }, { 51,554 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 13,1 }, { 1,344 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 13,1 }, { 42,889 }, { 7,7 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 13,1 }, { 9,344 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 13,1 }, { 17,344 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 13,1 }, { 9,326 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 13,1 }, { 1,325 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 13,1 }, { 25,344 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 7,1 }, { 8,306 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 7,1 }, { 1,305 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 7,1 }, { 15,306 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 7,1 }, { 30,309 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 7,1 }, { 17,326 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 7,1 }, { 22,305 }, { 5,8 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 7,1 }, { 1,334 }, { 6,8 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 7,1 }, { 44,310 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 7,1 }, { 51,310 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 7,1 }, { 8,296 }, { 5,8 }, 0, 0, false, false, 0 }
};  const size_t size_segment_d_and_j_climber = sizeof(segment_d_and_j_climber)/sizeof(segment_d_and_j_climber[0]); 
 const uint16_t segment_info_d_and_j_climber[] = { 64, 1024, 1, 0, 103, 240}; 


const std::string path_background_d_and_j_climber = "";
const uint16_t background_info_d_and_j_climber[] = { 128, 256, 0, 0, 103, 240, 1, 0, 0 }; 


const std::string path_console_d_and_j_climber = "romfs:/gfx/console_d_and_j_climber.t3x";
const uint16_t console_info_d_and_j_climber[] = { 512, 256, 0, 16, 320, 240}; 




const GW_rom d_and_j_climber (
    "David the climber", "d_and_j_climber", "2024-08-26"
    , rom_d_and_j_climber, size_rom_d_and_j_climber
    , melody_d_and_j_climber, size_melody_d_and_j_climber
    , path_segment_d_and_j_climber
    , segment_d_and_j_climber, size_segment_d_and_j_climber
    , segment_info_d_and_j_climber
    , path_background_d_and_j_climber
    , background_info_d_and_j_climber
    , path_console_d_and_j_climber
    , console_info_d_and_j_climber
    , GW_rom::MANUFACTURER_TRONICA
);

