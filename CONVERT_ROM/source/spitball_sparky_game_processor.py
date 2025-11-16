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
		self.colour_mode = "rows"
		self.colour_names = {0: "none", 1: "purple", 2: "teal", 3: "coral", 4: "blue"}
		self.header_fraction: float = 0.15
		self.add_colour_indices()

		updater = GamesPathUpdater()
		target = updater.get_target(self.game_key)

		# Combine the regular background with the Subtract, Overlay, Frame and Lines
		game_folder = target.background_paths[0].parent

		# Subtract
		subtract_path = game_folder / "Subtract.png"
		# check frame exists
		if not subtract_path.exists():
			print(f"Frame file not found: {subtract_path} for game {self.game_key}")
			raise SystemExit(1)
		
		combined_background = self.combine_backgrounds(target.background_paths[0], subtract_path, mode="multiply")
		target.background_paths = [combined_background]

		# Overlay
		overlay_path = game_folder / "Overlay.png"
		# check frame exists
		if not overlay_path.exists():
			print(f"Frame file not found: {overlay_path} for game {self.game_key}")
			raise SystemExit(1)
		
		combined_background = self.combine_backgrounds(target.background_paths[0], overlay_path, mode="add")
		target.background_paths = [combined_background]

		# Lines
		lines_path = game_folder / "Lines2.png"
		# check frame exists
		if not lines_path.exists():
			print(f"Frame file not found: {lines_path} for game {self.game_key}")
			raise SystemExit(1)
		
		combined_background = self.combine_backgrounds(target.background_paths[0], lines_path)
		target.background_paths = [combined_background]

		# Frame
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

