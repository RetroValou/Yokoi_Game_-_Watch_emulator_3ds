# Game processor for Crab Grab
from pathlib import Path
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor

class SpitballSparkyGameProcessor(GameProcessor):
	def __init__(self, target_name: str = "3ds"):
		super().__init__(target_name)

		# Specifics for Spitball Sparky
		self.game_key = "Spitball_Sparky" # Key from games_path.py
		self.game_folder = "gnw_ssparky" # Folder name for the game
		self.split_ratio_top = 0.4884  # Specific split ratio for Spitball Sparky

	def post_process(self):
		self.colour_mode = "rows"
		self.colour_names = {0: "none", 1: "purple", 2: "teal", 3: "coral", 4: "blue"}
		self.header_fraction: float = 0.15
		self.add_colour_indices()

		updater = GamesPathUpdater(self.target_name)
		target = updater.get_target(self.game_key)
		game_folder = target.background_paths[0].parent

		# Create the background by combining all the layers
		target.background_paths = self.combine_background_paths(target.background_paths[0], game_folder / "Subtract.png", mode="multiply")
		target.background_paths = self.combine_background_paths(target.background_paths[0], game_folder / "Overlay.png", mode="add")
		target.background_paths = self.combine_background_paths(target.background_paths[0], game_folder / "Lines2.png")
		target.background_paths = self.combine_background_paths(target.background_paths[0], game_folder / "Frame2.png")

		# Set custom values
		target.alpha_bright = 0.8
		target.fond_bright = 1.3
		target.color_segment = True
		target.shadow = False

		updater.write()


		self.multiscreen_conversion()

