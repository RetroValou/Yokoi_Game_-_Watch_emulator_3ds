"""Generate ``games_path`` entries directly from ``default.lay`` files."""

from __future__ import annotations

import re
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple

from analyze_default_lay import iter_artwork_folders, resolve_default_lay

# Preferred background views ordered by desirability. Boolean marks multi-screen views.
VIEW_PRIORITY: Sequence[Tuple[str, bool]] = (
    ("Background Only (No Frame)", False),
    ("Backgrounds Only (No Frame)", True),
    ("Background Only (No Shadow)", False),
    ("Backgrounds Only (No Shadow)", True),
    ("Background Only (No Reflection)", False),
    ("Backgrounds Only (No Reflection)", True),
    ("Background Only", False),
    ("Backgrounds Only", True),
)

INDENT_KEY_FIRST = "              "
INDENT_KEY_OTHER = "            , "
INDENT_FIELD = "                    "
INDENT_CONT = "                                    "
INDENT_FOOTER = "                }"

MODEL_PATTERN = re.compile(r"^[A-Za-z]{2,4}[-_]?\d{2,3}[A-Za-z0-9_-]*$")
DISPLAY_NAME_PATTERNS = (
    re.compile(r"<!--\s*Game\s*&\s*Watch:\s*([^-]+?)\s*-->", re.IGNORECASE),
    re.compile(r"<!--\s*([^:-]+?)\s*-->")
)

ROM_PRIORITY = ("*.program", "*.bin", "*.rom", "*.prg", "*.hex")


@dataclass(frozen=True)
class GameMetadata:
    model: str
    title: str
    display_title: str
    release_date: Optional[str]


@dataclass(frozen=True)
class Bounds:
    x: float
    y: float
    width: float
    height: float

    def as_int_tuple(self) -> Tuple[int, int, int, int]:
        return (
            int(round(self.x)),
            int(round(self.y)),
            int(round(self.width)),
            int(round(self.height)),
        )


@dataclass(frozen=True)
class SurfaceData:
    screen_bounds: Bounds
    background_bounds: Optional[Bounds]
    background_file: Optional[str]


def _strip_title_prefix(title: str) -> str:
    cleaned = title.strip()
    if not cleaned:
        return cleaned
    head, sep, tail = cleaned.partition(":")
    return tail.strip() if sep else cleaned


def _normalize_release_date(raw: str) -> Optional[str]:
    value = raw.strip()
    if not value:
        return None
    if value.count(".") == 2:
        parts = value.split(".")
        if all(part.isdigit() for part in parts):
            year, month, day = parts
            return f"{year.zfill(4)}-{month.zfill(2)}-{day.zfill(2)}"
    return value


def _load_game_metadata(script_dir: Path) -> Dict[str, GameMetadata]:
    md_path = script_dir / "GNW_LIST.md"
    try:
        text = md_path.read_text(encoding="utf-8")
    except OSError:
        return {}

    mapping: Dict[str, GameMetadata] = {}
    in_table = False
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped:
            in_table = False
            continue
        if stripped.startswith("| No.") and "Filename" in stripped:
            in_table = True
            continue
        if not in_table:
            continue
        if stripped.startswith("|---"):
            continue
        if not stripped.startswith("|"):
            in_table = False
            continue

        columns = [col.strip() for col in stripped.strip("|").split("|")]
        if len(columns) < 5:
            continue

        filename = columns[2]
        if not filename.lower().endswith(".zip"):
            continue

        model = columns[1]
        title = columns[3]
        release_raw = columns[4]

        base_name = filename.rsplit(".", 1)[0].lower()
        display_title = _strip_title_prefix(title)
        release_date = _normalize_release_date(release_raw)

        mapping[base_name] = GameMetadata(
            model=model.strip(),
            title=title.strip(),
            display_title=display_title,
            release_date=release_date,
        )

    return mapping


def _parse_float(value: str) -> Optional[float]:
    try:
        return float(value.strip())
    except (ValueError, AttributeError):
        return None


def _bounds_from_attrs(attrs: Dict[str, str]) -> Optional[Bounds]:
    if not attrs:
        return None

    x_val = _parse_float(attrs.get("x"))
    y_val = _parse_float(attrs.get("y"))
    width_val = _parse_float(attrs.get("width"))
    height_val = _parse_float(attrs.get("height"))

    if width_val is not None and height_val is not None:
        if x_val is None:
            x_val = 0.0
        if y_val is None:
            y_val = 0.0
        return Bounds(x_val, y_val, width_val, height_val)

    left_val = _parse_float(attrs.get("left"))
    right_val = _parse_float(attrs.get("right"))
    top_val = _parse_float(attrs.get("top"))
    bottom_val = _parse_float(attrs.get("bottom"))

    if None not in (left_val, right_val, top_val, bottom_val):
        return Bounds(left_val, top_val, right_val - left_val, bottom_val - top_val)

    return None


def _extract_bounds(node: Optional[ET.Element], root: ET.Element) -> Optional[Bounds]:
    if node is None:
        return None

    bounds_node = node.find("bounds")
    bounds = _bounds_from_attrs(bounds_node.attrib if bounds_node is not None else node.attrib)
    if bounds is not None:
        return bounds

    ref_name = node.get("ref") or node.get("element")
    if ref_name:
        ref_element = root.find(f".//element[@name='{ref_name}']")
        if ref_element is not None:
            ref_bounds_node = ref_element.find("bounds")
            ref_bounds = _bounds_from_attrs(
                ref_bounds_node.attrib if ref_bounds_node is not None else ref_element.attrib
            )
            if ref_bounds is not None:
                return ref_bounds

    return None


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

    def format_lines(self, script_dir: Path) -> List[str]:
        """Format this entry using the house style seen in ``convert_3ds.py``."""

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


def _extract_display_name(text: str) -> Optional[str]:
    for pattern in DISPLAY_NAME_PATTERNS:
        match = pattern.search(text)
        if match:
            val = match.group(1).strip()
            if val:
                return val
    return None


def _collect_element_images(root: ET.Element) -> Dict[str, str]:
    mapping: Dict[str, str] = {}
    for element in root.findall("element"):
        name_attr = element.get("name") or ""
        image_node = element.find("image")
        if image_node is not None and "file" in image_node.attrib:
            mapping[name_attr] = image_node.attrib["file"]
    return mapping


def _is_background_name(name: str) -> bool:
    lowered = name.lower()
    return (
        "background" in lowered
        or "backdrop" in lowered
        or lowered.startswith("bg")
        or lowered.startswith("screen")
    )


def _find_adjacent_background(
    siblings: Sequence[ET.Element],
    position: int,
) -> Optional[ET.Element]:
    before = position - 1
    while before >= 0:
        candidate = siblings[before]
        if candidate.tag == "screen":
            break
        if candidate.tag in {"element", "overlay"}:
            name = candidate.get("ref") or candidate.get("element") or ""
            if _is_background_name(name):
                return candidate
        before -= 1

    after = position + 1
    while after < len(siblings):
        candidate = siblings[after]
        if candidate.tag == "screen":
            break
        if candidate.tag in {"element", "overlay"}:
            name = candidate.get("ref") or candidate.get("element") or ""
            if _is_background_name(name):
                return candidate
        after += 1

    return None


def _resolve_background_file(node: Optional[ET.Element], image_map: Dict[str, str]) -> Optional[str]:
    if node is None:
        return None
    name_attr = node.get("ref") or node.get("element") or node.get("name") or ""
    if name_attr:
        file_name = image_map.get(name_attr)
        if file_name:
            return file_name
    file_attr = node.get("file")
    if file_attr:
        return file_attr
    image_child = node.find("image")
    if image_child is not None and "file" in image_child.attrib:
        return image_child.attrib["file"]
    return None


def _collect_background_candidates(
    siblings: Sequence[ET.Element],
    image_map: Dict[str, str],
) -> List[str]:
    collected: List[str] = []
    seen: set[str] = set()
    for node in siblings:
        if node.tag not in {"element", "overlay"}:
            continue
        name_attr = node.get("ref") or node.get("element") or ""
        if not _is_background_name(name_attr):
            continue
        file_name = image_map.get(name_attr)
        if file_name and file_name not in seen:
            collected.append(file_name)
            seen.add(file_name)
    return collected


def _extract_view_assets(
    root: ET.Element,
    image_map: Dict[str, str],
) -> Tuple[List[str], List[SurfaceData]]:
    for view_name, _ in VIEW_PRIORITY:
        view = root.find(f".//view[@name='{view_name}']")
        if view is None:
            continue

        siblings = list(view)
        screen_positions = [i for i, node in enumerate(siblings) if node.tag == "screen"]
        surfaces: List[SurfaceData] = []
        background_files: List[str] = []

        if screen_positions:
            ordered_ids: List[str] = []
            surface_by_id: Dict[str, SurfaceData] = {}
            for pos in screen_positions:
                screen_node = siblings[pos]
                screen_bounds = _extract_bounds(screen_node, root)
                if screen_bounds is None:
                    continue
                screen_id = screen_node.get("index") or str(len(ordered_ids))
                if screen_id not in surface_by_id:
                    ordered_ids.append(screen_id)
                background_node = _find_adjacent_background(siblings, pos)
                background_bounds = _extract_bounds(background_node, root)
                background_file = _resolve_background_file(background_node, image_map)
                surface_by_id[screen_id] = SurfaceData(
                    screen_bounds=screen_bounds,
                    background_bounds=background_bounds,
                    background_file=background_file,
                )

            surfaces = [surface_by_id[idx] for idx in ordered_ids]
            seen_files: set[str] = set()
            for surface in surfaces:
                if surface.background_file and surface.background_file not in seen_files:
                    background_files.append(surface.background_file)
                    seen_files.add(surface.background_file)

            if surfaces or background_files:
                return background_files, surfaces

        fallback_files = _collect_background_candidates(siblings, image_map)
        if fallback_files:
            return fallback_files, surfaces

    return [], []


def _compute_transform(surface: SurfaceData) -> List[List[int]]:
    screen = surface.screen_bounds
    bg = surface.background_bounds

    width = int(round(screen.width))
    height = int(round(screen.height))

    if bg is None:
        return [[width, 0, 0], [height, 0, 0]]

    left_cut = max(0, int(round(bg.x - screen.x)))
    right_cut = max(0, int(round((screen.x + screen.width) - (bg.x + bg.width))))
    top_cut = max(0, int(round(bg.y - screen.y)))
    bottom_cut = max(0, int(round((screen.y + screen.height) - (bg.y + bg.height))))

    return [[width, left_cut, right_cut], [height, top_cut, bottom_cut]]


def _compute_transforms(surfaces: Sequence[SurfaceData]) -> List[List[List[int]]]:
    return [_compute_transform(surface) for surface in surfaces]


def _find_rom_in_folder(folder: Path) -> Optional[Path]:
    for pattern in ROM_PRIORITY:
        matches = sorted(folder.glob(pattern))
        if matches:
            return matches[0]

    bare: List[Path] = []
    for item in folder.iterdir():
        if (
            item.is_file()
            and item.suffix == ""
            and item.name.lower() not in {"default", "default.lay", ".gitkeep"}
        ):
            bare.append(item)

    if bare:
        return sorted(bare)[0]
    return None


def _resolve_rom_file(
    name: str,
    folder: Path,
    fallback_folder: Optional[Path],
    folder_map: Dict[str, Path],
) -> Optional[Path]:
    rom_path = _find_rom_in_folder(folder)
    if rom_path is not None:
        return rom_path

    if fallback_folder is not None:
        rom_path = _find_rom_in_folder(fallback_folder)
        if rom_path is not None:
            return rom_path

    for fallback_name in _rom_fallback_candidates(name):
        candidate_folder = folder_map.get(fallback_name)
        if candidate_folder is None:
            continue
        rom_path = _find_rom_in_folder(candidate_folder)
        if rom_path is not None:
            return rom_path

    return None


def _find_melody_file(folder: Path, fallback: Optional[Path]) -> Optional[Path]:
    matches = sorted(folder.glob("*.melody"))
    if matches:
        return matches[0]
    if fallback is not None:
        matches = sorted(fallback.glob("*.melody"))
        if matches:
            return matches[0]
    return None


def _visual_sort_key(path: Path) -> Tuple[int, str]:
    lowered = path.name.lower()
    if any(token in lowered for token in ("top", "upper", "left")):
        priority = 0
    elif any(token in lowered for token in ("bottom", "lower", "right")):
        priority = 2
    else:
        priority = 1
    return priority, lowered


def _find_visual_files(folder: Path, fallback: Optional[Path]) -> List[Path]:
    visuals = sorted(folder.glob("*.svg"), key=_visual_sort_key)
    if visuals:
        return visuals
    if fallback is not None:
        return sorted(fallback.glob("*.svg"), key=_visual_sort_key)
    return []


def _resolve_console_path(folder: Path) -> Path:
    return folder / f"{folder.name}.png"


def _sanitize_key(display_name: str, fallback: str) -> str:
    base = re.sub(r"[^0-9A-Za-z]+", "_", display_name).strip("_")
    if not base:
        base = re.sub(r"[^0-9A-Za-z]+", "_", fallback).strip("_")
    if base and base[0].isdigit():
        base = f"_{base}"
    return base or fallback


def _rom_fallback_candidates(name: str) -> List[str]:
    candidates: List[str] = []

    if len(name) > 1:
        trimmed = name[:-1]
        if len(trimmed) >= 3:
            candidates.append(trimmed)

    if name == "gnw_egg":
        candidates.append("gnw_mmouse")
    if name == "gnw_dkcirc":
        candidates.append("gnw_mmousep")

    return candidates


def _parse_layout(lay_path: Path) -> Tuple[Optional[str], Optional[str], List[str], List[SurfaceData]]:
    try:
        text = lay_path.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        return None, None, [], []

    display_name = _extract_display_name(text)

    try:
        root = ET.fromstring(text)
    except ET.ParseError:
        return display_name, None, [], []

    image_map = _collect_element_images(root)
    background_files, surfaces = _extract_view_assets(root, image_map)

    ref_candidate: Optional[str] = None
    for element in root.findall("element"):
        name_attr = element.get("name") or ""
        if ref_candidate is None and MODEL_PATTERN.match(name_attr):
            ref_candidate = name_attr.strip()

    return display_name, ref_candidate, background_files, surfaces


def main() -> None:
    script_dir = Path(__file__).resolve().parent
    metadata_map = _load_game_metadata(script_dir)
    rom_root = script_dir / "rom"
    if not rom_root.exists():
        print("rom/ directory not found", file=sys.stderr)
        raise SystemExit(1)

    folder_map = {name: path for name, path in iter_artwork_folders(rom_root)}

    entries: List[GameEntry] = []
    skipped: List[Tuple[str, str]] = []
    key_counts: Dict[str, int] = {}
    missing_console: List[Path] = []

    for name, folder in folder_map.items():
        metadata = metadata_map.get(name.lower())
        layout_path, source = resolve_default_lay(name, folder, folder_map)
        fallback_folder = folder_map.get(source) if source not in {"", "self"} else None

        if layout_path is None:
            trimmed_name = name[:-1] if len(name) > 1 else ""
            trimmed_folder = folder_map.get(trimmed_name)
            if trimmed_folder is not None:
                layout_path = trimmed_folder / "default.lay"
                if layout_path.exists():
                    fallback_folder = trimmed_folder
            if layout_path is None or not layout_path.exists():
                skipped.append((name, "missing default.lay"))
                continue

        display_name, model_ref, background_files, surfaces = _parse_layout(layout_path)

        rom_path = _resolve_rom_file(name, folder, fallback_folder, folder_map)
        if rom_path is None:
            skipped.append((name, "no ROM candidate"))
            continue

        visuals = _find_visual_files(folder, fallback_folder)
        if not visuals:
            skipped.append((name, "no SVG visuals"))
            continue

        resolved_backgrounds: List[Path] = []
        for file_name in background_files:
            primary_candidate = folder / file_name
            if primary_candidate.exists():
                resolved_backgrounds.append(primary_candidate)
                continue
            if fallback_folder is not None:
                fallback_candidate = fallback_folder / file_name
                if fallback_candidate.exists():
                    resolved_backgrounds.append(fallback_candidate)
        background_paths = resolved_backgrounds

        transform_data = _compute_transforms(surfaces) if surfaces else []
        if transform_data and len(transform_data) < len(visuals):
            transform_data = []

        console_path = _resolve_console_path(folder)
        if not console_path.exists():
            missing_console.append(console_path)

        melody_path = _find_melody_file(folder, fallback_folder)

        rom_stem = rom_path.stem.strip()
        if rom_stem == "0019_238e":
            ref_value = "mg-8"
        elif metadata and metadata.model:
            ref_value = metadata.model.strip().lower()
        elif rom_stem:
            ref_value = rom_stem
        elif model_ref:
            ref_value = model_ref.strip()
        else:
            ref_value = ""

        display_source = metadata.display_title if metadata else _strip_title_prefix(display_name or "")
        key_candidate = _sanitize_key(display_source or name, name)
        index = key_counts.get(key_candidate, 0)
        key_counts[key_candidate] = index + 1
        key = key_candidate if index == 0 else f"{key_candidate}_{index + 1}"

        display_label = display_source or key_candidate.replace("_", " ")
        date_value = metadata.release_date if metadata else None

        entry = GameEntry(
            folder_name=name,
            key=key,
            display_name=display_label,
            ref=ref_value,
            rom_path=rom_path,
            visual_paths=visuals,
            background_paths=background_paths,
            console_path=console_path,
            melody_path=melody_path,
            date=date_value,
            transform_visual=transform_data,
        )
        entries.append(entry)

    entries.sort(key=lambda item: item.key.lower())

    output_lines: List[str] = ["games_path = {"]
    for idx, entry in enumerate(entries):
        prefix = INDENT_KEY_FIRST if idx == 0 else INDENT_KEY_OTHER
        output_lines.append(f'{prefix}"{entry.key}" :')
        output_lines.extend(entry.format_lines(script_dir))
        if idx != len(entries) - 1:
            output_lines.append(INDENT_KEY_FIRST)
    output_lines.append("}")

    destination = script_dir / "generated_games_path.py"
    destination.write_text("\n".join(output_lines) + "\n", encoding="utf-8")

    print(f"Generated {len(entries)} games. Output -> {destination}")
    if skipped:
        print("\nSkipped folders:")
        for name, reason in skipped:
            print(f"  - {name}: {reason}")
    if missing_console:
        print("\nMissing console images:")
        for path in sorted({p.resolve() for p in missing_console}):
            print(f"  - {path}")


if __name__ == "__main__":  # pragma: no cover - script entry point
    main()
