# Game prcessor for Crab Grab
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
        self.multiscreen_conversion()

