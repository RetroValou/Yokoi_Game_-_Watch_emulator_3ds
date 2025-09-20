# Yokoi - Game & Watch Emulator for 3DS
A Game & Watch emulator for 3DS: SM5A, SM510, and SM511/SM512.

# Build Instructions
The code has been tested and compiled on Windows. The conversion scripts may not work on Linux or Mac due to the reliance on Inkscape.  
The source code is provided **without the ROMs or graphics of the original Game & Watch devices** (copyrighted).  

To convert the ROMs, use the provided Python program included in the `CONVERT_ROM` folder. The ROMs used are the same as those that work with MAME.  
ROM parameters should be specified in the `games_path` dictionary. A pre-filled version is included in the source code.  
There is no strict ROM size limit—you can define one or use existing sizes.  

The conversion code requires **Inkscape**, as MAME ROMs use the SVG vector format (not directly readable on the 3DS). Inkscape is used to convert these vector graphics into PNG images.

# License
Public Domain / Free to use  
No attribution required :)

# Credits
Code inspired by MAME, Game & Watch FPGA projects (Adam Gastineau), and official Sharp documentation.  
ROMs and artwork are based on MAME ROMs.  
Thanks to everyone who contributed to MAME, Adam Gastineau, and all those who worked on Game & Watch ROMs and artwork!
