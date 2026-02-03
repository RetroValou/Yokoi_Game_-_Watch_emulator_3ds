# Game processor for Rain Shower
from platform import processor
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor


class RainShowerGameProcessor(GameProcessor):
	def __init__(self, target_name: str = "3ds"):
		super().__init__(target_name)

		# Specifics for Rain Shower
		self.game_key = "Rain_Shower"  # Key from games_path.py
		self.game_folder = "gnw_rshower"  # Folder name for the game

	def post_process(self):
		updater = GamesPathUpdater(self.target_name)
		target = updater.get_target(self.game_key)

		# Set custom values
		target.alpha_bright = 1.3
		target.fond_bright = 1.3

		updater.write()
