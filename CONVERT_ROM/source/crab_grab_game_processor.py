# Game processor for Crab Grab
from platform import processor
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
		self.colour_mode = "columns"
		self.colour_names = {0: "none", 1: "purple", 2: "mint", 3: "coral", 4: "blue"}
		self.header_fraction: float = 0.10
		self.add_colour_indices()

		# Combine the regular background with the Subtract, Overlay and Frame
		updater = GamesPathUpdater()
		target = updater.get_target(self.game_key)
		game_folder = target.background_paths[0].parent

		# Create the background by combining all the layers
		target.background_paths = self.combine_background_paths(target.background_paths[0], game_folder / "Subtract.png", mode="multiply")
		target.background_paths = self.combine_background_paths(target.background_paths[0], game_folder / "Overlay.png", mode="add")
		target.background_paths = self.combine_background_paths(target.background_paths[0], game_folder / "Frame2.png")

		# Set custom values
		target.alpha_bright = 0.8
		target.fond_bright = 1.3
		target.color_segment = True
		target.shadow = False

		updater.write()

		self.multiscreen_conversion()

