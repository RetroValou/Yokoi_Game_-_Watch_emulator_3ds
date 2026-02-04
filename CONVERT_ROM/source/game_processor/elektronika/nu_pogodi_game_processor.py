from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor


class NuPogodiGameProcessor(GameProcessor):
	def __init__(self, target_name: str = "3ds"):
		super().__init__(target_name)

		# Specifics for Nu, pogodi! (Elektronika)
		self.game_key = "Nu_pogodi"  # Key from games_path.py
		self.game_folder = "nupogodi"  # Folder name for the game

	def post_process(self):
		updater = GamesPathUpdater(self.target_name)
		target = updater.get_target(self.game_key)
		if target is None:
			return

		# Preserve white details in the background.
		target.background_keep_white = True
		target.background_white_keep_threshold = 180

		updater.write()
