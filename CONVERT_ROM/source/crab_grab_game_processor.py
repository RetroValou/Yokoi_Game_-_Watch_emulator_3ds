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

		# Subtract is in the same folder as background
		game_folder = target.background_paths[0].parent
		subtract_path = game_folder / "Subtract.png"
		# check subtract exists
		if not subtract_path.exists():
			print(f"Subtract file not found: {subtract_path} for game {self.game_key}")
			raise SystemExit(1)
		
		combined_background = self.combine_backgrounds(target.background_paths[0], subtract_path, mode="multiply")
		target.background_paths = [combined_background]

		# Overlay is in the same folder as background
		overlay_path = game_folder / "Overlay.png"
		# check overlay exists
		if not overlay_path.exists():
			print(f"Overlay file not found: {overlay_path} for game {self.game_key}")
			raise SystemExit(1)
		
		combined_background = self.combine_backgrounds(target.background_paths[0], overlay_path, mode="add")
		target.background_paths = [combined_background]

		# Frame is in the same folder as background
		frame_path = game_folder / "Frame2.png"
		# check frame exists
		if not frame_path.exists():
			print(f"Frame file not found: {frame_path} for game {self.game_key}")
			raise SystemExit(1)
		
		combined_background = self.combine_backgrounds(target.background_paths[0], frame_path)
		target.background_paths = [combined_background]

		# Set custom values
		target.alpha_bright = 0.8
		target.fond_bright = 1.3
		target.color_segment = True
		target.shadow = False

		updater.write()

		self.multiscreen_conversion()

