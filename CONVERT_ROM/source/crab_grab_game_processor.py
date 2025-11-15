# Game processor for Crab Grab
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor

class CrabGrabGameProcessor(GameProcessor):
    def __init__(self):
        super().__init__()

        # Specifics for Crab Grab
        self.game_key = "Crab_Grab" # Key from games_path.py
        self.game_folder = "gnw_cgrab" # Folder name for the game
        self.split_ratio_top = 0.4735  # Specific split ratio for Crab Grab

    def post_process(self):
        self.add_colour_indices()

        # Combine the regular background with the Frame with the pink link separating the header
        updater = GamesPathUpdater()
        target = updater.get_target(self.game_key)
        # Frame is in the same folder as background
        frame_path = target.background_paths[0].parent / "Frame2.png"
        # check frame exists
        if not frame_path.exists():
            print(f"Frame file not found: {frame_path} for game {self.game_key}")
            raise SystemExit(1)
        
        combined_background = self.combine_backgrounds(target.background_paths[0], frame_path)
        target.background_paths = [combined_background]

        # Set custom values
        target.alpha_bright = 1.0
        target.fond_bright = 1.3
        target.color_segment = True

        updater.write()

        self.multiscreen_conversion()

