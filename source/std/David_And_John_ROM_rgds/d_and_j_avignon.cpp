
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "d_and_j_avignon.h"


const uint8_t rom_GW_d_and_j_avignon[] = {
    0xFF, 0xFF, 0xFF, 0x02
}; const size_t size_rom_GW_d_and_j_avignon = sizeof(rom_GW_d_and_j_avignon)/sizeof(rom_GW_d_and_j_avignon[0]);

const uint8_t melody_GW_d_and_j_avignon[1] = {0}; 
	const size_t size_melody_GW_d_and_j_avignon = 0;

const std::string path_segment_d_and_j_avignon = "gfx/segment_d_and_j_avignon.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_d_and_j_avignon[] = {
	{ { 0,0,0 }, { 439,8 }, { 1,1756 }, { 70,90 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 411,83 }, { 76,1742 }, { 49,99 }, 0, 0, false, false, 0 }, { { 0,0,10 }, { 86,86 }, { 1,1178 }, { 63,96 }, 0, 0, false, false, 0 }, { { 0,0,11 }, { 131,204 }, { 1,1506 }, { 85,72 }, 0, 0, false, false, 0 }, { { 0,0,12 }, { 23,19 }, { 1,1276 }, { 70,87 }, 0, 0, false, false, 0 }, { { 0,0,13 }, { 0,86 }, { 1,713 }, { 46,96 }, 0, 0, false, false, 0 }, { { 0,0,14 }, { 10,221 }, { 1,1051 }, { 96,55 }, 0, 0, false, false, 0 }, { { 0,0,15 }, { 514,88 }, { 76,1646 }, { 51,94 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 435,204 }, { 1,1432 }, { 85,72 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 329,21 }, { 1,1580 }, { 73,85 }, 0, 0, false, false, 0 }, { { 0,0,4 }, { 297,83 }, { 73,1843 }, { 53,99 }, 0, 0, false, false, 0 }, { { 0,0,5 }, { 329,211 }, { 1,1365 }, { 94,65 }, 0, 0, false, false, 0 }, { { 0,0,6 }, { 228,17 }, { 1,1667 }, { 72,87 }, 0, 0, false, false, 0 }, { { 0,0,7 }, { 198,86 }, { 66,1178 }, { 57,96 }, 0, 0, false, false, 0 }, { { 0,0,8 }, { 223,208 }, { 1,1108 }, { 86,68 }, 0, 0, false, false, 0 }, { { 0,0,9 }, { 131,12 }, { 1,1848 }, { 70,94 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 443,183 }, { 1,675 }, { 103,36 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 443,183 }, { 1,486 }, { 103,35 }, 0, 0, false, false, 0 }, { { 0,1,10 }, { 133,183 }, { 1,449 }, { 103,35 }, 0, 0, false, false, 0 }, { { 0,1,11 }, { 131,172 }, { 1,1003 }, { 105,46 }, 0, 0, false, false, 0 }, { { 0,1,12 }, { 29,183 }, { 1,637 }, { 103,36 }, 0, 0, false, false, 0 }, { { 0,1,13 }, { 29,183 }, { 1,412 }, { 103,35 }, 0, 0, false, false, 0 }, { { 0,1,14 }, { 27,172 }, { 1,955 }, { 105,46 }, 0, 0, false, false, 0 }, { { 0,1,15 }, { 0,183 }, { 97,1563 }, { 29,35 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 441,172 }, { 1,907 }, { 105,46 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 340,183 }, { 1,599 }, { 103,36 }, 0, 0, false, false, 0 }, { { 0,1,4 }, { 340,183 }, { 1,375 }, { 103,35 }, 0, 0, false, false, 0 }, { { 0,1,5 }, { 338,172 }, { 1,859 }, { 105,46 }, 0, 0, false, false, 0 }, { { 0,1,6 }, { 236,183 }, { 1,561 }, { 103,36 }, 0, 0, false, false, 0 }, { { 0,1,7 }, { 236,183 }, { 1,338 }, { 103,35 }, 0, 0, false, false, 0 }, { { 0,1,8 }, { 234,172 }, { 1,811 }, { 105,46 }, 0, 0, false, false, 0 }, { { 0,1,9 }, { 133,183 }, { 1,523 }, { 103,36 }, 0, 0, false, false, 0 }, { { 0,10,0 }, { 538,23 }, { 1,1944 }, { 100,103 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 504,55 }, { 97,1467 }, { 29,30 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 400,55 }, { 97,1531 }, { 30,30 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 297,55 }, { 97,1435 }, { 29,30 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 193,55 }, { 97,1499 }, { 30,30 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 90,55 }, { 97,1403 }, { 29,30 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 577,129 }, { 49,752 }, { 61,57 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 551,0 }, { 1,284 }, { 62,52 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 519,228 }, { 76,1600 }, { 42,44 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 558,228 }, { 73,1319 }, { 42,44 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 596,228 }, { 65,292 }, { 42,44 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 618,202 }, { 112,905 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 618,202 }, { 109,374 }, { 12,19 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 618,202 }, { 112,884 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 618,202 }, { 112,863 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 618,202 }, { 109,689 }, { 16,19 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 618,202 }, { 112,842 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 618,202 }, { 112,821 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 618,202 }, { 112,800 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 618,202 }, { 112,779 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 618,202 }, { 112,758 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 601,202 }, { 109,647 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 601,202 }, { 109,353 }, { 12,19 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 601,202 }, { 109,626 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 601,202 }, { 109,605 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 601,202 }, { 109,668 }, { 16,19 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 601,202 }, { 109,584 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 601,202 }, { 109,563 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 601,202 }, { 109,542 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 601,202 }, { 109,521 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 601,202 }, { 109,500 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 583,202 }, { 112,1157 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 583,202 }, { 109,395 }, { 13,19 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 583,202 }, { 112,1136 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 583,202 }, { 112,1115 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 583,202 }, { 109,731 }, { 17,19 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 583,202 }, { 112,1094 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 583,202 }, { 112,1073 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 583,202 }, { 112,1052 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 583,202 }, { 112,1031 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 583,202 }, { 112,1010 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 566,202 }, { 109,479 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 566,202 }, { 109,332 }, { 12,19 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 566,202 }, { 112,989 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 566,202 }, { 112,968 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 566,202 }, { 109,710 }, { 17,19 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 566,202 }, { 112,947 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 566,202 }, { 112,926 }, { 15,19 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 566,202 }, { 109,458 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 566,202 }, { 109,437 }, { 14,19 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 566,202 }, { 109,416 }, { 14,19 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_avignon = sizeof(segment_GW_d_and_j_avignon)/sizeof(segment_GW_d_and_j_avignon[0]); 
 const uint16_t segment_info_d_and_j_avignon[] = { 128, 2048, 1, 0, 638, 276}; 

const std::string path_background_d_and_j_avignon = "";
const uint16_t background_info_d_and_j_avignon[] = { 1024, 512, 0, 0, 638, 276, 1, 0, 0 }; 


const std::string path_console_d_and_j_avignon = "gfx/console_d_and_j_avignon.png";
const uint16_t console_info_d_and_j_avignon[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom d_and_j_avignon (
    "Vacation in Avignon", "D_AND_J_AVIGNON", "2024-08-26"
    , rom_GW_d_and_j_avignon, size_rom_GW_d_and_j_avignon
    , melody_GW_d_and_j_avignon, size_melody_GW_d_and_j_avignon
    , path_segment_d_and_j_avignon
    , segment_GW_d_and_j_avignon, size_segment_GW_d_and_j_avignon
    , segment_info_d_and_j_avignon
    , path_background_d_and_j_avignon
    , background_info_d_and_j_avignon
    , path_console_d_and_j_avignon
    , console_info_d_and_j_avignon
    , GW_rom::MANUFACTURER_DAVID_AND_JOHN
);

