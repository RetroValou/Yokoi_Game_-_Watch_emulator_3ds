"""Post-processing helpers for individual games.

This module encapsulates per-game post-processing in a simple object-
oriented API. The main entry point is :class:`GameProcessor`, which is
seeded with a single game key (e.g. ``"gnw_cgrab"``), can load its
information from ``games_path.py``, and exposes helpers to split
background images and SVGs.
"""

from pathlib import Path
from typing import Optional, Tuple, List

from PIL import Image
import xml.etree.ElementTree as ET
import re
import copy

from source.games_path_utils import GameEntry, GamesPathUpdater, write_games_path, load_games_path 


def _get_group_y_position(g_elem: ET.Element) -> Optional[float]:
	"""Extract Y from the first path's moveto command in a <g>.

	This matches the approach used for the Crab Grab SVG, but is generic
	enough to work for other games with similar layering.
	"""

	for el in g_elem.iter():
		if el.tag.endswith("path"):
			d = el.get("d", "") or ""
			m = re.search(r"[mM]\s*[-\d.]+[ ,]+([-\d.]+)", d)
			if m:
				try:
					return float(m.group(1))
				except ValueError:
					return None
	return None


class GameProcessor:
	"""Post-processing helper for a single game."""

	def __init__(self) -> None:
		# Overrides by subclasses for specific games
		self.game_key = ""  # Game key from games_path.py	
		self.game_folder = ""  # Folder name for the game
		self.split_ratio_top = 0.50  # Default, set this in the specific game subclass

		self.script_root = Path(__file__).parent.parent
		self.rom_root = self.script_root / "rom"
		self.info: Optional[GameEntry] = None

		# Colour mapping configuration (can be overridden per-game)
		# mode: 'columns' -> vertical bands based on X, 'rows' -> horizontal bands based on Y
		self.colour_mode: str = "columns"
		# Optional explicit band mapping: index -> colour name, mainly for logging
		self.colour_names = {0: "none", 1: "purple", 2: "mint", 3: "coral", 4: "blue"}
		# Fraction of total height considered as header region for colour index 0
		self.header_fraction: float = 0.10

	def load_info(self) -> bool:
		updater = GamesPathUpdater()
		target = updater.get_target(self.game_key)
		if target is None:
			print(f"Game '{self.game_key}' not found in games_path")
			return False
		
		self.info = target

		# folder path for this game
		self.base_dir = self.rom_root / self.game_folder
		self.rework_dir = self.base_dir / "rework" # output rework folder
		
		return True

	def add_colour_indices(self) -> None:
		"""Add colour indices to this game's SVG segments based on position.

		This is a genericised version of the Crab Grab-specific
		``add_colour_codes`` logic. It reads the base SVG from this game's
		folder and writes a processed copy with indices into the ``rework``
		subfolder, preserving the original filename.
		"""

		# Ensure rework directory exists
		self.rework_dir.mkdir(parents=True, exist_ok=True)

		if not self.info.visual_paths:
			print("Cannot add colour indices: 'Visual' field missing in games_path entry.")
			return

		# First visual path for this game
		in_svg = self.info.visual_paths[0]
		out_svg = self.rework_dir / in_svg.name

		if not in_svg.exists():
			print(f"ERROR: Input SVG not found: {in_svg}")
			return

		print(f"\nProcessing (add_colour_indices): {in_svg.name}")

		tree = ET.parse(in_svg)
		root = tree.getroot()

		viewbox = root.get("viewBox", "0 0 2084 3696")
		parts = viewbox.split()
		viewbox_width = float(parts[2]) if len(parts) >= 3 else 2084.0
		viewbox_height = float(parts[3]) if len(parts) >= 4 else 3696.0

		print(f"ViewBox width: {viewbox_width}")
		print(f"ViewBox height: {viewbox_height}")

		# Register namespaces to preserve them
		ns_map = {
			"": "http://www.w3.org/2000/svg",
			"sodipodi": "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd",
			"inkscape": "http://www.inkscape.org/namespaces/inkscape",
			"dc": "http://purl.org/dc/elements/1.1/",
			"cc": "http://creativecommons.org/ns#",
			"rdf": "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
		}
		for prefix, uri in ns_map.items():
			ET.register_namespace(prefix, uri)

		modified_count = 0
		skipped_count = 0

		def get_segment_xy_position(g_element: ET.Element) -> tuple[Optional[float], Optional[float]]:
			"""Extract approximate X and Y from the first path in a group."""
			for path in g_element.iter():
				if path.tag.endswith("path"):
					d = path.get("d", "")
					m = re.search(r"[mM]\s*([\-\d.]+)[,\s]+([\-\d.]+)", d)
					if m:
						x = float(m.group(1))
						y = float(m.group(2))
						return x, y
			return None, None

		def determine_column_index(x_pos: Optional[float], viewbox_width: float) -> int:
			"""Determine colour index from X position (4 vertical columns)."""
			if x_pos is None:
				return 0

			band_width = viewbox_width / 4.0

			if x_pos < band_width:
				return 1
			elif x_pos < band_width * 2:
				return 2
			elif x_pos < band_width * 3:
				return 3
			else:
				return 4

		def determine_row_index(y_pos: Optional[float], viewbox_height: float) -> int:
			"""Determine colour index from Y position (5-band layout including header).

			Bands as fractions of full height:
			- Header (cream):     ~20% -> index 0 (handled by is_in_header)
			- Band 1 (coral):     ~12% -> index 3
			- Band 2 (blue):      ~18% -> index 4
			- Band 3 (teal):      ~18% -> index 2
			- Band 4 (purple):    ~32% -> index 1
			"""
			if y_pos is None:
				return 0

			h = viewbox_height

			header_end = h * self.header_fraction          # 15%
			band1_end  = h * (self.header_fraction + 0.10) # 10%
			band2_end  = h * (self.header_fraction + 0.10 + 0.14) # 50%
			band3_end  = h * (self.header_fraction + 0.10 + 0.14 + 0.20) # 68%
			# remainder (68%–100%) is band 4

			# Note: header check is done separately in is_in_header,
			# so anything below header_end should be in one of the colour bands.
			if y_pos < header_end:
				return 0  # safety; normally caught by is_in_header
			elif y_pos < band1_end:
				return 3  # coral
			elif y_pos < band2_end:
				return 4  # blue
			elif y_pos < band3_end:
				return 2  # teal
			else:
				return 1  # purple

		def is_in_header(y_pos: Optional[float], viewbox_height: float) -> bool:
			"""Return True if segment is in the header.

			Uses per-game configurable self.header_fraction of the total height.
			"""
			if y_pos is None:
				return False
			header_threshold = viewbox_height * self.header_fraction
			return y_pos < header_threshold

		for elem in root.iter():
			if not elem.tag.endswith("g"):
				continue

			# Find title in group
			title_elem = None
			for child in elem:
				if child.tag.endswith("title"):
					title_elem = child
					break

			if title_elem is None or title_elem.text is None:
				continue

			current_title = title_elem.text.strip()

			# Already has index?
			if "_" in current_title:
				skipped_count += 1
				continue

			x_pos, y_pos = get_segment_xy_position(elem)
			if x_pos is None and y_pos is None:
				print(f"	Warning: no position for {current_title}")
				continue

			if is_in_header(y_pos, viewbox_height):
				color_idx = 0
				reason = "in header"
			else:
				# Choose banding mode based on configured colour_mode
				if self.colour_mode == "rows":
					color_idx = determine_row_index(y_pos, viewbox_height)
				else:
					color_idx = determine_column_index(x_pos, viewbox_width)

				if color_idx == 0:
					print(f"	Warning: could not assign colour for {current_title}")
					continue
				reason = None

			new_title = f"{current_title}_{color_idx}"
			title_elem.text = new_title

			colour_names = self.colour_names or {}
			extra = f" ({reason})" if reason else ""
			print(
				f"	{current_title} (x={x_pos:.1f}, y={y_pos:.1f}) "
				f"-> {new_title} ({colour_names.get(color_idx, 'unknown')}){extra}"
			)
			modified_count += 1

		tree.write(out_svg, encoding="UTF-8", xml_declaration=True)

		# Update games_path.py so this game's Visual entry points at the
		# newly-generated SVG in the rework folder.
		self._update_games_path_visual(out_svg)

		print(
			f"\nSummary: {modified_count} segments updated, "
			f"{skipped_count} skipped (already had index)"
		)

	def combine_images(self, image1: Path, image2: Path, mode: str = "normal") -> Path:
		"""Combine two background images into one with a blend mode.

		Args:
			bg1: Base/background image path (destination).
			bg2: Foreground image path (source).
			mode: One of "normal" (alpha over), "add", or "multiply".

		Returns:
			Path to the combined image written into the rework folder.
		"""
		out_filename = f"{image1.stem}_combined{image1.suffix}"
		out_path = self.rework_dir / out_filename

		img1 = Image.open(image1).convert("RGBA")
		img2 = Image.open(image2).convert("RGBA")

		if img2.size != img1.size:
			img2 = img2.resize(img1.size, resample=Image.BILINEAR)

		mode = mode.lower()
		combined = img1.copy()

		if mode == "normal":
			# Standard source-over alpha composite
			combined.alpha_composite(img2)
		elif mode in {"add", "multiply"}:
			# Use ImageChops for per-channel math; preserve alpha from img2
			from PIL import ImageChops

			base_rgb = combined.convert("RGB")
			top_rgb = img2.convert("RGB")
			if mode == "add":
				result_rgb = ImageChops.add(base_rgb, top_rgb, scale=1.0, offset=0)
			else:  # "multiply"
				result_rgb = ImageChops.multiply(base_rgb, top_rgb)

			# Reattach alpha (use top image alpha to mimic layout semantics)
			result = Image.new("RGBA", combined.size)
			alpha = img2.split()[-1]
			rgba_data = [
				(r, g, b, a)
				for (r, g, b), a in zip(result_rgb.getdata(), alpha.getdata())
			]
			result.putdata(rgba_data)
			combined = result
		else:
			# Fallback to normal if an unknown mode is provided
			combined.alpha_composite(img2)

		combined.save(out_path)

		width, height = combined.size
		print(f"Saved combined background: {out_path.name} ({width}x{height})")

		return out_path

	def combine_background_paths(self, current_background: Path, overlay_background: Path, mode:str="normal"):
		# check overlay_background exists
		if not overlay_background.exists():
			print(f"File not found: {overlay_background} for game {self.game_key}")
			raise SystemExit(1)
		
		return [self.combine_images(current_background, overlay_background, mode=mode)]
	
	def multiscreen_conversion(self) -> None:
		"""Placeholder for multiscreen conversion logic for this game."""
		updater = GamesPathUpdater()
		target = updater.get_target(self.game_key)

		if target is None:
			print(f"Cannot update games_path.py: entry '{self.game_key}' not found.")
			return
		if target.background_paths is None or len(target.background_paths) != 1:
			print("Cannot split background: 'Background' field missing or invalid in games_path entry.")
			return
		if target.visual_paths is None or len(target.visual_paths) != 1:
			print("Cannot split SVG: 'Visual' field missing or invalid in games_path entry.")
			return

		svg_top, svg_bottom = self.split_svg(target.visual_paths[0])
		background_top, background_bottom = self.split_background(target.background_paths[0])

		target.visual_paths = [svg_top, svg_bottom]
		target.background_paths = [background_top, background_bottom]
		
		# Split the transform_visual entries
		if target.transform_visual and len(target.transform_visual) == 1:
			original_transform = target.transform_visual[0]
			# Half the height element
			original_transform[1][0] = int(original_transform[1][0] * 0.50) # Use 50% because the screens are the same height
			target.transform_visual = [original_transform, original_transform]

		# Add the size_visual for multiscreen
		target.size_visual = [[320,240],[320,240]]

		try:
			updater.write(self.script_root)
			print(f"Updated Visual and Background entries for '{self.game_key}' in games_path.py")
		except Exception as e:
			print(f"Error updating games_path.py: {e}")


	def split_background(
		self,
		background_source: str,
	) -> None:
		"""Split this game's background image into top and bottom parts."""

		top_path = background_source.with_name(background_source.stem + "_top" + background_source.suffix)
		bottom_path = background_source.with_name(background_source.stem + "_bottom" + background_source.suffix)

		img = Image.open(background_source)
		width, height = img.size
		print(f"{self.base_dir.name} background size: {width}x{height}")

		split_row = int(round(height * self.split_ratio_top))

		# Top part
		img_up = img.crop((0, 0, width, split_row))
		img_up.save(top_path)
		print(f"Saved {top_path.name}: {width}x{split_row}")

		# Bottom part
		img_down = img.crop((0, split_row, width, height))
		img_down.save(bottom_path)
		print(f"Saved {bottom_path.name}: {width}x{height - split_row}")

		return top_path, bottom_path

	def split_svg(
		self,
		svg_source: Path,
		segs_layer_label: str = "segs",
		background_layer_label: str = "white",
	) -> Tuple[Path, Path]:
		"""Split this game's SVG into top and bottom halves.

		This mirrors the behaviour of ``split_cgrab_svg``: the background
		layer (typically ``"white"``) is preserved in both halves, while the
		segment layer (typically ``"segs"``) is divided based on Y position.
		"""

		# Build the destination paths for top and bottom SVGs by appending
		# ``_top``/``_bottom`` to the original filename.
		top_svg = svg_source.with_name(svg_source.stem + "_top" + svg_source.suffix)
		bottom_svg = svg_source.with_name(svg_source.stem + "_bottom" + svg_source.suffix)

		tree = ET.parse(svg_source)
		root = tree.getroot()

		viewbox = root.get("viewBox", "0 0 2084 3696")
		parts = viewbox.split()
		vb_x, vb_y, vb_w, vb_h = map(float, parts)

		split_y = vb_h * self.split_ratio_top
		print(f"{self.base_dir.name} SVG viewBox height: {vb_h}, split_y: {split_y}")

		# Find layers
		layer_background = None
		layer_segs = None
		for g in root:
			if not g.tag.endswith("g"):
				continue
			label = g.get("{http://www.inkscape.org/namespaces/inkscape}label", "")
			if label == background_layer_label:
				layer_background = g
			elif label == segs_layer_label:
				layer_segs = g

		root_top = copy.deepcopy(root)
		root_bottom = copy.deepcopy(root)

		root_top.set("viewBox", f"{vb_x} {vb_y} {vb_w} {split_y}")
		root_bottom.set("viewBox", f"{vb_x} {split_y} {vb_w} {vb_h - split_y}")

		root_top.set("height", f"{split_y}pt")
		root_bottom.set("height", f"{vb_h - split_y}pt")

		def find_layer(root_copy: ET.Element, label: str) -> Optional[ET.Element]:
			for g in root_copy:
				if g.tag.endswith("g") and g.get(
					"{http://www.inkscape.org/namespaces/inkscape}label", ""
				) == label:
					return g
			return None

		segs_top = find_layer(root_top, segs_layer_label)
		segs_bottom = find_layer(root_bottom, segs_layer_label)

		if segs_top is None or segs_bottom is None or layer_segs is None:
			print(
				f"ERROR: Could not locate '{segs_layer_label}' layer in SVG for {self.base_dir.name}; "
				"skipping split."
			)
			return top_svg, bottom_svg

		# Clear and reallocate only the segs layer; the background layer
		# remains intact in both outputs.
		segs_top[:] = []
		segs_bottom[:] = []

		for g in list(layer_segs):
			if not g.tag.endswith("g"):
				continue
			y = _get_group_y_position(g)
			target = segs_top if (y is not None and y < split_y) else segs_bottom
			import copy as _copy

			target.append(_copy.deepcopy(g))

		ET.ElementTree(root_top).write(top_svg, encoding="UTF-8", xml_declaration=True)
		ET.ElementTree(root_bottom).write(bottom_svg, encoding="UTF-8", xml_declaration=True)
		print(f"Saved split SVGs for {self.base_dir.name}: {top_svg.name}, {bottom_svg.name}")

		return top_svg, bottom_svg

	def _load_games_path(self) -> Tuple[Path, List[GameEntry]]:
		"""Load the games_path entries and return them together with the path.

		Raises RuntimeError on failure.
		"""

		return load_games_path()

	def _update_games_path_visual(self, out_svg: Path) -> None:
		"""Replace this game's Visual entry in games_path.py with ``out_svg``"""
		updater = GamesPathUpdater()

		# games_path values are relative paths rooted at script_root.
		# Keep that convention for the new visual path.
		rel_visual = out_svg.resolve().relative_to(self.script_root)
		target = updater.get_target(self.game_key)
		if target is None:
			print(f"Cannot update games_path.py: entry '{self.game_key}' not found.")
			return

		target.visual_paths = [rel_visual]
		try:
			updater.write(self.script_root)
			print(f"Updated Visual entry for '{self.game_key}' in games_path.py")
		except Exception as exc:
			print(f"Failed to write updated games_path.py: {exc}")

