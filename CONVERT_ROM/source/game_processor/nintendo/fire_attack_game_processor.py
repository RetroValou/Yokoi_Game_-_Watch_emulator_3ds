# Game processor for Fire Attack
from platform import processor
from source.games_path_utils import GamesPathUpdater
from source.game_processor import GameProcessor


class FireAttackGameProcessor(GameProcessor):
	def __init__(self, target_name: str = "3ds"):
		super().__init__(target_name)

		# Specifics for Fire Attack
		self.game_key = "Fire_Attack"  # Key from games_path.py
		self.game_folder = "gnw_fireatk"  # Folder name for the game

	def post_process(self):
		updater = GamesPathUpdater(self.target_name)
		target = updater.get_target(self.game_key)

		# Set custom values
		target.alpha_bright = 1.1
		target.fond_bright = 1.2

		updater.write()
