# Segment format

{ segment_id, pos_screen, pos_in_texture, size_in_texture, color_index, screen, ?, ?, ?}

- segment_id : compose to 3 value, this is used to determine which segment should be activated based on whether the CPU SMxxx pins are on or off
- pos_screen : where the segment should be displayed on the screen 
- pos_in_texture : where the segment image is positioned on the texture containing all the segments.
- size_in_texture : size of segment on the texture
- color_index : in most case 0. Used for G&W with colored segment (crab grab and Spitball Sparky )
- screen : screen id who segment need to show
- ?
- ?
- ?

example : { { 0,0,1 }, { 267,17 }, { 328,4 }, { 5,10 }, 0, 0, false, false, 0 }