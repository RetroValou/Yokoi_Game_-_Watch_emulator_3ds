# Game processor for Crab Grab
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor

class SpitballSparkyGameProcessor(GameProcessor):
    def __init__(self):
        super().__init__()

        # Specifics for Spitball Sparky
        self.game_key = "Spitball_Sparky" # Key from games_path.py
        self.game_folder = "gnw_ssparky" # Folder name for the game
        self.split_ratio_top = 0.4884  # Specific split ratio for Spitball Sparky

    def post_process(self):
        self.add_colour_indices()

        updater = GamesPathUpdater()
        target = updater.get_target(self.game_key)

        # Combine the regular background with the Frame 
        # Frame is in the same folder as background
        game_folder = target.background_paths[0].parent
        frame_path = game_folder / "Frame2.png"
        # check frame exists
        if not frame_path.exists():
            print(f"Frame file not found: {frame_path} for game {self.game_key}")
            raise SystemExit(1)
        
        combined_background = self.combine_backgrounds(target.background_paths[0], frame_path)
        target.background_paths = [combined_background]

        # Combine the background and Frame combo background with the Lines background
        # Lines is in the same folder as background
        lines_path = game_folder / "Lines2.png"
        # check frame exists
        if not lines_path.exists():
            print(f"Frame file not found: {lines_path} for game {self.game_key}")
            raise SystemExit(1)
        
        combined_background = self.combine_backgrounds(target.background_paths[0], lines_path)
        target.background_paths = [combined_background]

        # Set custom values
        target.alpha_bright = 1
        target.fond_bright = 1.3
        target.color_segment = True

        updater.write()


        self.multiscreen_conversion()

