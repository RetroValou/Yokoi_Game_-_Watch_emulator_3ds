"""Shared helpers for building and writing ``games_path``.

This module centralises the ``GameEntry`` data structure and the
formatting logic used to generate ``games_path.py`` so that both
``generate_games_path.py`` and ``game_processor.py`` can keep the file
layout consistent.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence, Tuple


INDENT_KEY_FIRST = "              "
INDENT_KEY_OTHER = "            , "
INDENT_FIELD = "                    "
INDENT_CONT = "                                    "
INDENT_FOOTER = "                }"

default_alpha_bright = 1.7
default_fond_bright = 1.35

@dataclass
class GameEntry:
	"""Structured data for one ``games_path`` dictionary entry."""

	folder_name: str
	key: str
	display_name: str
	ref: str
	rom_path: Path
	visual_paths: List[Path]
	background_paths: List[Path]
	console_path: Optional[Path]
	melody_path: Optional[Path]
	date: Optional[str] = None
	transform_visual: List[List[List[int]]] = field(default_factory=list)
	size_visual: List[List[int]] = field(default_factory=list)
	two_in_one_screen: bool = False
	mask: bool = False
	color_segment: bool = False
	alpha_bright: float = default_alpha_bright
	fond_bright: float = default_fond_bright
	rotate: bool = False

	def format_lines(self, script_dir: Path) -> List[str]:
		"""Format this entry using the house style used by the generator."""

		lines: List[str] = []
		lines.append(f'{INDENT_FIELD}{{ "ref" : "{self.ref}"')
		lines.append(f'{INDENT_FIELD}, "display_name" : "{self.display_name}"')
		lines.append(f'{INDENT_FIELD}, "Rom" : {_format_path(self.rom_path, script_dir)}')

		if self.melody_path is not None:
			lines.append(
				f'{INDENT_FIELD}, "Melody_Rom" : {_format_path(self.melody_path, script_dir)}'
			)

		lines.extend(_format_path_list("Visual", self.visual_paths, script_dir))
		lines.extend(_format_path_list("Background", self.background_paths, script_dir))
		lines.append(
			f'{INDENT_FIELD}, "transform_visual" : {_format_transform(self.transform_visual)}'
		)

		if self.size_visual:
			lines.append(
				f'{INDENT_FIELD}, "size_visual" : '
				f'{_format_transform([self.size_visual])[1:-1]}'
			)

		if self.two_in_one_screen:
			lines.append(f'{INDENT_FIELD}, "2_in_one_screen" : True')

		if self.mask:
			lines.append(f'{INDENT_FIELD}, "mask" : True')

		if self.color_segment:
			lines.append(f'{INDENT_FIELD}, "color_segment" : True')

		# Only emit non-default brightness / rotation values to keep
		# backwards compatibility with existing games_path entries.
		if self.alpha_bright != default_alpha_bright:
			lines.append(f'{INDENT_FIELD}, "alpha_bright" : {self.alpha_bright}')

		if self.fond_bright != default_fond_bright:
			lines.append(f'{INDENT_FIELD}, "fond_bright" : {self.fond_bright}')

		if self.rotate:
			lines.append(f'{INDENT_FIELD}, "rotate" : True')

		if self.date is not None:
			lines.append(f'{INDENT_FIELD}, "date" : "{self.date}"')

		if self.console_path is not None:
			lines.append(
				f'{INDENT_FIELD}, "console" : {_format_path(self.console_path, script_dir)}'
			)

		lines.append(INDENT_FOOTER)
		return lines


def _format_transform(transform: Sequence[Sequence[Sequence[int]]]) -> str:
	if not transform:
		return "[]"
	return str(transform).replace(" ", "")


def _format_path(path: Path, script_dir: Path) -> str:
	rel = path.resolve().relative_to(script_dir)
	windows_style = str(rel).replace("/", "\\")
	return f"r'.\\{windows_style}'"


def _format_path_list(
	label: str,
	paths: Sequence[Path],
	script_dir: Path,
) -> List[str]:
	if not paths:
		return [f'{INDENT_FIELD}, "{label}" : []']

	rendered = [f'{INDENT_FIELD}, "{label}" : [{_format_path(paths[0], script_dir)}']
	for path in paths[1:]:
		rendered.append(f'{INDENT_CONT}, {_format_path(path, script_dir)}')
	rendered[-1] += ']'
	return rendered

def load_games_path() -> Tuple[Path, List[GameEntry]]:
	"""Dynamically load ``games_path.py`` from its fixed location.

	Returns a tuple of (path_to_games_path_py, entries).
	Raises ``RuntimeError`` if the file is missing or malformed.
	"""

	# games_path.py lives one level above this source/ folder.
	script_root = Path(__file__).resolve().parent.parent
	games_path_module = script_root / "games_path.py"
	if not games_path_module.exists():
		raise RuntimeError(f"games_path.py not found at {games_path_module}")

	import importlib.util

	spec = importlib.util.spec_from_file_location("games_path", str(games_path_module))
	if spec is None or spec.loader is None:
		raise RuntimeError("Unable to create import spec for games_path.py")

	module = importlib.util.module_from_spec(spec)
	spec.loader.exec_module(module)  # type: ignore[assignment]
	games_path = getattr(module, "games_path", None)
	if not isinstance(games_path, dict):
		raise RuntimeError("games_path.py does not define a 'games_path' dict")

	entries = _dict_to_entries(games_path, script_root)
	return games_path_module, entries

def build_games_path_lines(entries: Sequence[GameEntry], script_dir: Path) -> List[str]:
	"""Build the full ``games_path.py`` file contents as a list of lines."""

	output_lines: List[str] = ["games_path = {"]
	for idx, entry in enumerate(entries):
		prefix = INDENT_KEY_FIRST if idx == 0 else INDENT_KEY_OTHER
		output_lines.append(f'{prefix}"{entry.key}" :')
		output_lines.extend(entry.format_lines(script_dir))
		if idx != len(entries) - 1:
			output_lines.append(INDENT_KEY_FIRST)
	output_lines.append("}")
	return output_lines


def write_games_path(entries: Sequence[GameEntry], script_dir: Path, destination: Path) -> None:
	"""Write ``games_path.py`` for the given entries to ``destination``.

	This is a thin convenience wrapper around :func:`build_games_path_lines`.
	"""

	lines = build_games_path_lines(entries, script_dir)
	destination.write_text("\n".join(lines) + "\n", encoding="utf-8")


class GamesPathUpdater:
	"""Helper for loading, updating, and rewriting ``games_path.py``.

	Typical usage::

		updater = GamesPathUpdater()
		entry = updater.get_target("gnw_cgrab")
		if entry is not None:
			# mutate entry in place
			entry.visual_paths = [Path("rom/gnw_cgrab/rework/foo.svg")]
			updater.write()

	The class keeps the entries list and target module path in memory so
	that multiple updates can be applied before a single call to
	:meth:`write`.
	"""

	def __init__(self) -> None:
		self.games_path_file, self.entries = load_games_path()

	def get_target(self, game_key: str) -> Optional[GameEntry]:
		"""Return the :class:`GameEntry` for ``game_key``, or ``None``."""

		for entry in self.entries:
			if entry.key == game_key:
				return entry
		return None

	def write(self, script_root: Optional[Path] = None) -> None:
		"""Rewrite ``games_path.py`` with the current entries.

		If ``script_root`` is omitted, the directory containing
		``games_path.py`` is used as the root for relative paths.
		"""

		if script_root is None:
			script_root = self.games_path_file.parent

		write_games_path(self.entries, script_root, self.games_path_file)


def _dict_to_entries(games_path: Dict[str, Any], script_root: Path) -> List[GameEntry]:
	"""Convert a ``games_path`` dict back into ``GameEntry`` objects.

	This is used by tools that want to rewrite ``games_path.py`` using
	the same formatting logic as the generator after modifying entries.
	"""

	entries: List[GameEntry] = []

	def _to_path(value: Any) -> Path:
		# Stored paths are raw strings like r'.\\rom\\gnw_ball\\file.ext'.
		if not isinstance(value, str):
			return script_root / "rom" / "MISSING"
		clean = value.lstrip(".\\/")
		return script_root / clean.replace("\\", "/")

	for key, data in games_path.items():
		if not isinstance(data, dict):
			continue

		rom_path = _to_path(data.get("Rom"))
		visual_values = data.get("Visual") or []
		background_values = data.get("Background") or []
		console_value = data.get("console")
		melody_value = data.get("Melody_Rom")

		visual_paths = [_to_path(p) for p in visual_values]
		background_paths = [_to_path(p) for p in background_values]
		console_path = _to_path(console_value) if console_value else None
		melody_path = _to_path(melody_value) if melody_value else None

		transform_visual = data.get("transform_visual") or []
		size_visual = data.get("size_visual") or []
		two_in_one = bool(data.get("2_in_one_screen", False))
		mask = bool(data.get("mask", False))
		color_segment = bool(data.get("color_segment", False))
		alpha_bright = float(data.get("alpha_bright", default_alpha_bright))
		fond_bright = float(data.get("fond_bright", default_fond_bright))
		rotate = bool(data.get("rotate", False))
		date = data.get("date")
		display_name = data.get("display_name", key)
		ref = data.get("ref", "")
		folder_name = data.get("folder_name", key)

		entries.append(
			GameEntry(
				folder_name=folder_name,
				key=key,
				display_name=display_name,
				ref=ref,
				rom_path=rom_path,
				visual_paths=visual_paths,
				background_paths=background_paths,
				console_path=console_path,
				melody_path=melody_path,
				date=date,
				transform_visual=transform_visual,
				size_visual=size_visual,
				two_in_one_screen=two_in_one,
				mask=mask,
				color_segment=color_segment,
				alpha_bright=alpha_bright,
				fond_bright=fond_bright,
				rotate=rotate,
			),
		)

	entries.sort(key=lambda e: e.key.lower())
	return entries


