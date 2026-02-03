# Game processor for Pinball
from platform import processor
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor


class PinballGameProcessor(GameProcessor):
	def __init__(self, target_name: str = "3ds"):
		super().__init__(target_name)

		# Specifics for Pinball
		self.game_key = "Pinball"  # Key from games_path.py
		self.game_folder = "gnw_pinball"  # Folder name for the game

	def post_process(self):
		updater = GamesPathUpdater(self.target_name)
		target = updater.get_target(self.game_key)

		# Set custom values
		target.alpha_bright = 1.2

		updater.write()
