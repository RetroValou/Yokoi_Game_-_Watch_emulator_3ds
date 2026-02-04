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
# 2. Generate games_path data to process files (includes numeric manufacturer id)
# 3. Post process any games if needed (e.g. Crab Grab, Spit Sparky)
# 4. Files and config now ready to run convert_3ds.py to generate build data in 3DS format

import argparse

from source.extract_assets import extract_assets
from source.generate_games_path import generate_games_path
from source.game_processor.nintendo.crab_grab_game_processor import CrabGrabGameProcessor
from source.game_processor.nintendo.spitball_sparky_game_processor import SpitballSparkyGameProcessor
from source.game_processor.tronica.space_adventure_game_processor import SpaceAdventureGameProcessor
from source.game_processor.tronica.spider_tronica_game_processor import SpiderTronicaGameProcessor
from source.game_processor.tronica.diver_s_adventure_game_processor import DiversAdventureGameProcessor
from source.game_processor.elektronika.kosmicheskiy_most_game_processor import KosmicheskiyMostGameProcessor
from source.game_processor.elektronika.kosmicheskiy_polyot_game_processor import KosmicheskiyPolyotGameProcessor
from source.game_processor.elektronika.hockey_game_processor import HockeyGameProcessor
from source.game_processor.elektronika.biathlon_game_processor import BiathlonGameProcessor
from source.game_processor.elektronika.ataka_asteroidov_game_processor import AtakaAsteroidovGameProcessor
from source.game_processor.elektronika.nu_pogodi_game_processor import NuPogodiGameProcessor
from source.game_processor.nintendo.turtle_bridge_game_processor import TurtleBridgeGameProcessor
from source.game_processor.nintendo.fire_attack_game_processor import FireAttackGameProcessor
from source.game_processor.nintendo.oil_panic_game_processor import OilPanicGameProcessor
from source.game_processor.nintendo.rain_shower_game_processor import RainShowerGameProcessor
from source.game_processor.nintendo.life_boat_game_processor import LifeBoatGameProcessor
from source.game_processor.nintendo.pinball_game_processor import PinballGameProcessor


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Prepare extracted artwork + generate games_path for a given target. "
            "Then apply any per-game post-processing (Crab Grab, Spitball Sparky)."
        )
    )
    parser.add_argument(
        "--target",
        default="3ds",
        help="Target profile to use for metadata sizing (e.g. '3ds' or 'rgds').",
    )
    args = parser.parse_args()

    extract_assets()

    if generate_games_path(args.target) is False:
        print("Missing files, check previous output for details")
        return 1

    # Optional: post-process specific games that need special handling.
    processors = [CrabGrabGameProcessor(args.target), SpitballSparkyGameProcessor(args.target), 
                  SpaceAdventureGameProcessor(args.target), SpiderTronicaGameProcessor(args.target),
                  DiversAdventureGameProcessor(args.target), KosmicheskiyMostGameProcessor(args.target), 
                  KosmicheskiyPolyotGameProcessor(args.target), HockeyGameProcessor(args.target),
                  BiathlonGameProcessor(args.target), AtakaAsteroidovGameProcessor(args.target),
                  NuPogodiGameProcessor(args.target),
                  TurtleBridgeGameProcessor(args.target), FireAttackGameProcessor(args.target),
                  OilPanicGameProcessor(args.target), RainShowerGameProcessor(args.target),
                  LifeBoatGameProcessor(args.target), PinballGameProcessor(args.target)]
    for processor in processors:
        if processor.load_info():
            processor.post_process()

    print("\nFiles processed and ready.")
    print("You can now run convert_3ds.py to generate build data for your target.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
