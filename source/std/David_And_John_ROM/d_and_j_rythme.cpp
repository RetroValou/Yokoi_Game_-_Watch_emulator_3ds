
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
	{ { 0,12,0 }, { 37,98 }, { 87,671 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,1 }, { 108,98 }, { 87,642 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,10 }, { 79,170 }, { 20,663 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,11 }, { 150,170 }, { 42,663 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,2 }, { 179,98 }, { 20,634 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,3 }, { 8,98 }, { 42,634 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,4 }, { 79,98 }, { 64,632 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,5 }, { 150,98 }, { 86,613 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,6 }, { 37,170 }, { 1,605 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,7 }, { 108,170 }, { 23,605 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,8 }, { 179,170 }, { 45,603 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,12,9 }, { 8,170 }, { 1,576 }, { 20,27 }, 0, 0, false, false, 0 }, { { 0,14,0 }, { 0,64 }, { 52,854 }, { 65,29 }, 0, 0, false, false, 0 }, { { 0,14,1 }, { 71,64 }, { 1,823 }, { 65,29 }, 0, 0, false, false, 0 }, { { 0,14,2 }, { 142,64 }, { 1,792 }, { 65,29 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 67,100 }, { 123,883 }, { 3,140 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 139,100 }, { 111,547 }, { 2,140 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 63,25 }, { 35,692 }, { 31,34 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 33,25 }, { 89,910 }, { 32,34 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 4,25 }, { 1,693 }, { 32,34 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 53,6 }, { 115,609 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 53,6 }, { 112,1009 }, { 9,14 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 53,6 }, { 115,593 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 53,6 }, { 115,577 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 53,6 }, { 115,673 }, { 12,14 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 53,6 }, { 115,561 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 53,6 }, { 115,545 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 53,6 }, { 67,616 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 53,6 }, { 1,638 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 53,6 }, { 89,894 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 40,6 }, { 102,894 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 40,6 }, { 112,993 }, { 9,14 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 40,6 }, { 45,558 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 40,6 }, { 58,558 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 40,6 }, { 115,657 }, { 12,14 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 40,6 }, { 71,568 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 40,6 }, { 84,568 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 40,6 }, { 97,568 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 40,6 }, { 71,552 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 40,6 }, { 84,552 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 27,6 }, { 97,552 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 27,6 }, { 112,977 }, { 9,14 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 27,6 }, { 68,536 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 27,6 }, { 81,536 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 27,6 }, { 115,641 }, { 12,14 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 27,6 }, { 94,536 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 27,6 }, { 107,529 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 27,6 }, { 68,520 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 27,6 }, { 81,520 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 27,6 }, { 94,520 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 14,6 }, { 107,513 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 14,6 }, { 112,961 }, { 9,14 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 14,6 }, { 1,516 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 14,6 }, { 14,516 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 14,6 }, { 115,625 }, { 12,14 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 14,6 }, { 27,516 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 14,6 }, { 40,516 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 14,6 }, { 53,516 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 14,6 }, { 66,504 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 14,6 }, { 79,504 }, { 11,14 }, 0, 0, false, false, 0 }, { { 0,0,0 }, { 3,108 }, { 1,946 }, { 56,77 }, 0, 1, false, false, 0 }, { { 0,0,1 }, { 74,107 }, { 1,866 }, { 49,78 }, 0, 1, false, false, 0 }, { { 0,0,2 }, { 148,108 }, { 59,946 }, { 51,77 }, 0, 1, false, false, 0 }, { { 0,1,0 }, { 1,147 }, { 1,654 }, { 17,37 }, 0, 1, false, false, 0 }, { { 0,1,1 }, { 72,147 }, { 109,689 }, { 18,37 }, 0, 1, false, false, 0 }, { { 0,1,2 }, { 184,147 }, { 68,661 }, { 17,37 }, 0, 1, false, false, 0 }, { { 0,10,0 }, { 71,210 }, { 68,700 }, { 39,26 }, 0, 1, false, false, 0 }, { { 0,11,0 }, { 61,215 }, { 123,878 }, { 3,3 }, 0, 1, false, false, 0 }, { { 0,11,1 }, { 61,209 }, { 45,547 }, { 11,9 }, 0, 1, false, false, 0 }, { { 0,11,2 }, { 61,205 }, { 92,505 }, { 11,13 }, 0, 1, false, false, 0 }, { { 0,12,12 }, { 37,1 }, { 23,576 }, { 20,27 }, 0, 1, false, false, 0 }, { { 0,12,13 }, { 108,1 }, { 67,584 }, { 20,27 }, 0, 1, false, false, 0 }, { { 0,12,14 }, { 179,1 }, { 45,574 }, { 20,27 }, 0, 1, false, false, 0 }, { { 0,12,15 }, { 8,1 }, { 89,584 }, { 20,27 }, 0, 1, false, false, 0 }, { { 0,12,16 }, { 79,1 }, { 1,547 }, { 20,27 }, 0, 1, false, false, 0 }, { { 0,12,17 }, { 150,1 }, { 23,547 }, { 20,27 }, 0, 1, false, false, 0 }, { { 0,13,0 }, { 33,32 }, { 68,791 }, { 29,61 }, 0, 1, false, false, 0 }, { { 0,13,1 }, { 105,32 }, { 99,791 }, { 28,61 }, 0, 1, false, false, 0 }, { { 0,13,2 }, { 176,32 }, { 32,729 }, { 28,61 }, 0, 1, false, false, 0 }, { { 0,13,3 }, { 3,32 }, { 62,728 }, { 28,61 }, 0, 1, false, false, 0 }, { { 0,13,4 }, { 74,32 }, { 92,728 }, { 28,61 }, 0, 1, false, false, 0 }, { { 0,13,5 }, { 145,32 }, { 1,729 }, { 29,61 }, 0, 1, false, false, 0 }, { { 0,2,0 }, { 67,0 }, { 115,885 }, { 3,23 }, 0, 1, false, false, 0 }, { { 0,2,1 }, { 139,0 }, { 119,860 }, { 2,23 }, 0, 1, false, false, 0 }, { { 0,3,0 }, { 0,61 }, { 1,542 }, { 65,3 }, 0, 1, false, false, 0 }, { { 0,3,1 }, { 71,61 }, { 1,537 }, { 65,3 }, 0, 1, false, false, 0 }, { { 0,3,2 }, { 142,61 }, { 1,532 }, { 65,3 }, 0, 1, false, false, 0 }, { { 0,4,0 }, { 114,181 }, { 52,885 }, { 35,59 }, 0, 1, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_rythme = sizeof(segment_GW_d_and_j_rythme)/sizeof(segment_GW_d_and_j_rythme[0]); 
 const uint16_t segment_info_d_and_j_rythme[] = { 128, 1024, 1, 0, 207, 240, 207, 240}; 

const std::string path_background_d_and_j_rythme = "";
const uint16_t background_info_d_and_j_rythme[] = { 256, 512, 0, 0, 207, 240, 0, 0, 207, 240, 1, 0, 0 }; 


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

