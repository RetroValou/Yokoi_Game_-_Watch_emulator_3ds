# Convert the original artwork and roms for MAME into the format for the 3DS Yokoi emulator
#
#  Note: You all need a screen shot of the console for each game to be used as the console image
#        which is not included in the original MAME artwork/roms. Easiest way to generate these 
#        is to run the game in MAME on the PC and take a screenshot of the game when it 
#        first starts. Put each of the screenshots into the rom/console_screenshots folder
#        with the filename matching the rom zip name, e.g. gnw_cgrab.png for gnw_cgrab.zip.
#
# Steps:
# =================================================
# 1. Extract roms from rom/artwork and rom/roms folder into rom/gnw_<game> folders
# 2. Generate games_path data to process files
# 3. Post process any games if needed (e.g. Crab Grab, Spit Sparky)
# 4. Files and config now ready to run convert_3ds.py to generate build data in 3DS format

from source.extract_assets import extract_assets
from source.generate_games_path import generate_games_path
from source.crab_grab_game_processor import CrabGrabGameProcessor
from source.spitball_sparky_game_processor import SpitballSparkyGameProcessor

extract_assets()

if generate_games_path() is False:
    print("Missing files, check previous output for details")
    exit(1)

# Optional: post-process specific games that need special handling.
processors = [CrabGrabGameProcessor(), SpitballSparkyGameProcessor()]
for processor in processors:
    if processor.load_info():
        processor.post_process()

print("\nFiles processed and ready.\n\nYou can now run convert_3ds.py to generate the 3DS build data.")
