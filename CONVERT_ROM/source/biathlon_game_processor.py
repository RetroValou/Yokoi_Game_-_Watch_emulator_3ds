# Game processor for Crab Grab
from platform import processor
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor

class BiathlonGameProcessor(GameProcessor):
	def __init__(self, target_name: str = "3ds"):
		super().__init__(target_name)

		# Specifics for Biathlon
		self.game_key = "Biathlon" # Key from games_path.py
		self.game_folder = "biathlon" # Folder name for the game

	def post_process(self):
		updater = GamesPathUpdater(self.target_name)
		target = updater.get_target(self.game_key)

		# Set custom values
		target.alpha_bright = 0.8
		target.fond_bright = 1.3

		updater.write()
