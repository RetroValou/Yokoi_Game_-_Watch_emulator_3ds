"""Utility to inspect Game & Watch artwork folders.

Currently prints a table listing every `gnw_*` directory (and `trshutvoy`)
inside `rom/`, indicating whether that folder contains a `default.lay`
layout file. This is a first step toward rebuilding `games_path`
information automatically from the raw assets.
"""

from pathlib import Path
import xml.etree.ElementTree as ET
from typing import Dict, Iterable, List, Optional, Tuple

TABLE_HEADERS = [
    "Folder",
    "Default.lay",
    "Source",
    "Background View",
    "BG Type",
    "Multiscreen",
    "View Bounds",
    "Screen Bounds",
    "BG Element",
    "BG Element Bounds",
    "BG Image Files",
]


def _stringify_rows(
    rows: Iterable[Tuple[str, bool, str, bool, bool, str, str, str, str, str, str]]
) -> List[List[str]]:
    string_rows: List[List[str]] = []
    for (
        name,
        present,
        source,
        has_background,
        is_multiscreen,
        bg_type,
        view_bounds,
        screen_bounds,
        bg_element,
        bg_element_bounds,
        bg_image_files,
    ) in rows:
        string_rows.append(
            [
                name,
                "Yes" if present else "No",
                source if source else "-",
                "Yes" if has_background else "No",
                bg_type if bg_type else "-",
                "Yes" if is_multiscreen else "No",
                view_bounds if view_bounds else "-",
                screen_bounds if screen_bounds else "-",
                bg_element if bg_element else "-",
                bg_element_bounds if bg_element_bounds else "-",
                bg_image_files if bg_image_files else "-",
            ]
        )
    return string_rows


def iter_artwork_folders(root: Path) -> Iterable[Tuple[str, Path]]:
    """Yield (name, path) for every gnw_* folder plus trshutvoy."""
    for entry in sorted(root.iterdir()):
        if not entry.is_dir():
            continue
        name = entry.name
        if name.startswith("gnw_") or name.lower() == "trshutvoy":
            yield name, entry


def resolve_default_lay(
    name: str,
    folder: Path,
    folder_map: Dict[str, Path],
) -> Tuple[Optional[Path], str]:
    """Check for default.lay in folder, falling back to the shortened name.

    Returns a tuple (layout_path, source_name) where:
      * layout_path: path to default.lay when found, otherwise None.
      * source_name: 'self' when the file lives in the folder itself,
        otherwise the fallback folder name that supplied the layout.
    """

    layout_path = folder / "default.lay"
    if layout_path.exists():
        return layout_path, "self"

    fallback_name = name[:-1]
    if len(fallback_name) < 3:  # avoid empty or trivial fallbacks
        return None, ""

    fallback_folder = folder_map.get(fallback_name)
    if fallback_folder:
        fallback_path = fallback_folder / "default.lay"
        if fallback_path.exists():
            return fallback_path, fallback_name

    return None, ""


def print_table(
    rows: Iterable[
        Tuple[str, bool, str, bool, bool, str, str, str, str, str, str]
    ]
) -> None:
    """Render a simple text table from the gathered data."""

    row_list = list(rows)
    string_rows = _stringify_rows(row_list)

    widths: List[int] = [len(header) for header in TABLE_HEADERS]
    for row_values in string_rows:
        for idx, value in enumerate(row_values):
            widths[idx] = max(widths[idx], len(value))

    def format_row(values: List[str]) -> str:
        return " | ".join(
            f"{value:<{widths[idx]}}" for idx, value in enumerate(values)
        )

    print(format_row(TABLE_HEADERS))
    separator = "-+-".join("-" * width for width in widths)
    print(separator)

    for values in string_rows:
        print(format_row(values))


def write_markdown_table(
    rows: Iterable[
        Tuple[str, bool, str, bool, bool, str, str, str, str, str, str]
    ],
    summary_lines: Iterable[str],
    output_path: Path,
) -> None:
    """Persist the table and summary to a Markdown file for easy viewing."""

    row_list = list(rows)
    string_rows = _stringify_rows(row_list)

    lines: List[str] = []
    header_row = "| " + " | ".join(TABLE_HEADERS) + " |"
    separator_row = "| " + " | ".join("---" for _ in TABLE_HEADERS) + " |"
    lines.append(header_row)
    lines.append(separator_row)
    for values in string_rows:
        lines.append("| " + " | ".join(values) + " |")

    summary_list = list(summary_lines)
    if summary_list:
        lines.append("")
        lines.append("### Summary")
        lines.append("")
        lines.extend(summary_list)

    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    rom_root = Path("rom")
    if not rom_root.exists():
        raise SystemExit("rom/ directory not found next to analyze_transforms.py")

    folder_map: Dict[str, Path] = {name: folder for name, folder in iter_artwork_folders(rom_root)}

    folder_rows: List[
        Tuple[str, bool, str, bool, bool, str, str, str, str, str, str]
    ] = []
    layout_paths: Dict[str, Optional[Path]] = {}
    fallback_uses = 0
    for name, folder in folder_map.items():
        layout_path, source = resolve_default_lay(name, folder, folder_map)
        layout_paths[name] = layout_path
        present = layout_path is not None
        if present and source != "self":
            fallback_uses += 1
        folder_rows.append(
            (name, present, source, False, False, "", "", "", "", "", "")
        )

    # Keep table sorted alphabetically by name for readability
    folder_rows.sort(key=lambda row: row[0])

    view_priority: List[Tuple[str, bool, str]] = [
        ("Background Only (No Frame)", False, "NF"),
        ("Backgrounds Only (No Frame)", True, "NF"),
        ("Background Only (No Shadow)", False, "NS"),
        ("Backgrounds Only (No Shadow)", True, "NS"),
        ("Background Only (No Reflection)", False, "NR"),
        ("Backgrounds Only (No Reflection)", True, "NR"),
        ("Background Only", False, "BG"),
        ("Backgrounds Only", True, "BG"),
    ]

    def format_bounds_from_attrs(attrs: Dict[str, str]) -> str:
        keys_in_order = (
            "x",
            "y",
            "width",
            "height",
            "left",
            "right",
            "top",
            "bottom",
        )
        parts = [f"{key}={attrs[key]}" for key in keys_in_order if key in attrs]
        if not parts:
            # Preserve any other coordinates that might appear
            parts = [f"{key}={value}" for key, value in attrs.items()]
        return " ".join(parts)

    def extract_bounds(node: Optional[ET.Element]) -> str:
        if node is None:
            return ""
        if node.tag == "bounds":
            return format_bounds_from_attrs(node.attrib)
        child = node.find("bounds")
        if child is not None:
            return format_bounds_from_attrs(child.attrib)
        return format_bounds_from_attrs(node.attrib)

    def detect_background_view(
        path: Path,
    ) -> Tuple[bool, bool, str, str, str, str, str, str]:
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            return False, False, "", "", "", "", "", ""

        try:
            root = ET.fromstring(text)
        except ET.ParseError:
            for view_name, is_multiscreen, bg_type in view_priority:
                if f'<view name="{view_name}"' in text:
                    return True, is_multiscreen, bg_type, "", "", "", "", ""
            return False, False, "", "", "", "", "", ""

        element_images: Dict[str, str] = {}
        for element in root.findall("element"):
            name_attr = element.get("name")
            if not name_attr:
                continue
            image_node = element.find("image")
            if image_node is not None and "file" in image_node.attrib:
                element_images[name_attr] = image_node.attrib["file"]

        def is_background_name(name: str) -> bool:
            lowered = name.lower()
            return (
                "background" in lowered
                or "backdrop" in lowered
                or lowered.startswith("bg")
                or lowered.startswith("screen")
            )

        def find_adjacent_background(
            children_list: List[ET.Element], start_pos: int
        ) -> Optional[ET.Element]:
            pos = start_pos - 1
            while pos >= 0:
                candidate = children_list[pos]
                if candidate.tag == "screen":
                    break
                if candidate.tag in {"element", "overlay"}:
                    name_attr = candidate.get("ref") or candidate.get("element") or ""
                    if is_background_name(name_attr):
                        return candidate
                pos -= 1
            pos = start_pos + 1
            while pos < len(children_list):
                candidate = children_list[pos]
                if candidate.tag == "screen":
                    break
                if candidate.tag in {"element", "overlay"}:
                    name_attr = candidate.get("ref") or candidate.get("element") or ""
                    if is_background_name(name_attr):
                        return candidate
                pos += 1
            return None

        for view_name, is_multiscreen, bg_type in view_priority:
            view = root.find(f".//view[@name='{view_name}']")
            if view is None:
                continue

            view_bounds = extract_bounds(view.find("bounds"))

            children = list(view)
            screen_entries: List[Tuple[str, str, int, ET.Element]] = []
            for position, child in enumerate(children):
                if child.tag != "screen":
                    continue
                bounds_str = extract_bounds(child)
                index_attr = child.get("index")
                label = index_attr if index_attr not in (None, "") else str(len(screen_entries))
                screen_entries.append((label, bounds_str, position, child))

            screen_bounds_parts: List[str] = []
            for label, bounds_str, _, _ in screen_entries:
                if bounds_str and label:
                    screen_bounds_parts.append(f"{label}:{bounds_str}")
                elif bounds_str:
                    screen_bounds_parts.append(bounds_str)
                elif label:
                    screen_bounds_parts.append(label)
            screen_bounds = "; ".join(screen_bounds_parts)

            bg_entries: List[Tuple[str, str, str, str]] = []

            def record_background(label: str, node: Optional[ET.Element]) -> None:
                if node is None:
                    bg_entries.append((label, "", "", "-"))
                    return
                name_attr = node.get("ref") or node.get("element") or ""
                bounds_str = extract_bounds(node)
                image_file = element_images.get(name_attr, "-")
                bg_entries.append((label, name_attr, bounds_str, image_file))

            if screen_entries:
                for order, (label, _, position, _) in enumerate(screen_entries):
                    label_to_use = label or str(order)
                    background_node = find_adjacent_background(children, position)
                    record_background(label_to_use, background_node)
            else:
                fallback_node: Optional[ET.Element] = None
                for child in children:
                    if child.tag in {"element", "overlay"}:
                        name_attr = child.get("ref") or child.get("element") or ""
                        if is_background_name(name_attr):
                            fallback_node = child
                            break
                record_background("", fallback_node)

            bg_element_names_output: List[str] = []
            bg_element_bounds_output: List[str] = []
            bg_image_files_output: List[str] = []
            for label, name_attr, bounds_str, image_file in bg_entries:
                label_prefix = f"{label}:" if label else ""
                display_name = name_attr if name_attr else "-"
                display_bounds = bounds_str if bounds_str else "-"
                display_image = image_file if image_file else "-"
                bg_element_names_output.append(
                    f"{label_prefix}{display_name}" if label_prefix else display_name
                )
                bg_element_bounds_output.append(
                    f"{label_prefix}{display_bounds}" if label_prefix else display_bounds
                )
                bg_image_files_output.append(
                    f"{label_prefix}{display_image}" if label_prefix else display_image
                )

            bg_element_name = "; ".join(bg_element_names_output)
            bg_element_bounds = "; ".join(bg_element_bounds_output)
            bg_image_files_str = "; ".join(bg_image_files_output)

            return (
                True,
                is_multiscreen,
                bg_type,
                view_bounds,
                screen_bounds,
                bg_element_name,
                bg_element_bounds,
                bg_image_files_str,
            )

        for view_name, is_multiscreen, bg_type in view_priority:
            if f'<view name="{view_name}"' in text:
                return True, is_multiscreen, bg_type, "", "", "", "", ""

        return False, False, "", "", "", "", "", ""

    for index, row in enumerate(folder_rows):
        name, present, source, *_ = row
        layout_path = layout_paths.get(name)
        (
            has_background,
            is_multiscreen,
            bg_type,
            view_bounds,
            screen_bounds,
            bg_element_name,
            bg_element_bounds,
            bg_image_files,
        ) = (
            detect_background_view(layout_path)
            if layout_path is not None
            else (False, False, "", "", "", "", "", "")
        )
        folder_rows[index] = (
            name,
            present,
            source,
            has_background,
            is_multiscreen,
            bg_type,
            view_bounds,
            screen_bounds,
            bg_element_name,
            bg_element_bounds,
            bg_image_files,
        )

    total = len(folder_rows)
    with_lay = sum(1 for row in folder_rows if row[1])
    without_lay = total - with_lay
    with_background_view = sum(1 for row in folder_rows if row[3])
    multiscreen_backgrounds = sum(1 for row in folder_rows if row[3] and row[4])

    markdown_summary: List[str] = [
        f"- Total folders: {total}",
        f"- With default.lay: {with_lay}",
        f"- Without default.lay: {without_lay}",
        f"- With background view: {with_background_view}",
        f"- Multiscreen background views: {multiscreen_backgrounds}",
    ]
    if fallback_uses:
        markdown_summary.append(f"- Using fallback layouts: {fallback_uses}")

    print_table(folder_rows)

    print("\nSummary:")
    print(f"  Total folders : {total}")
    print(f"  With default.lay : {with_lay}")
    print(f"  Without default.lay : {without_lay}")
    print(
        "  With background view : "
        f"{with_background_view}"
    )
    print(
        "  Of those, multiscreen background : "
        f"{multiscreen_backgrounds}"
    )
    if fallback_uses:
        print(f"  Using fallback layouts : {fallback_uses}")

    output_path = Path(__file__).resolve().parent / "default.lays.md"
    write_markdown_table(folder_rows, markdown_summary, output_path)
    print(f"\nMarkdown report written to {output_path}")


if __name__ == "__main__":
    main()
