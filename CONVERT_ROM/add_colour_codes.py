"""Utilities for colouring Crab Grab SVG segments and combining background/frame.

- Input SVG:  CONVERT_ROM/rom/gnw_cgrab/gnw_cgrab.svg
- Output SVG: CONVERT_ROM/rom/gnw_cgrab/gnw_cgrab_coloured.svg
- Background/frame:
        - Background base: Background2.png (chosen in layout)
        - Overlay frame:   Frame2.png (contains pink divider line)
        - Output combined: Background2_with_frame.png

Colour bands for crabs (left → right columns):
    column 0: purple  -> index 1
    column 1: mint    -> index 2
    column 2: coral   -> index 3
    column 3: blue    -> index 4
"""

import re
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image


def get_segment_xy_position(g_element):
    """Extract approximate X and Y from the first path in a group."""
    for path in g_element.iter():
        if path.tag.endswith("path"):
            d = path.get("d", "")
            # First moveto command: m x,y or M x,y
            m = re.search(r"[mM]\s*([-\d.]+)[,\s]+([-\d.]+)", d)
            if m:
                x = float(m.group(1))
                y = float(m.group(2))
                return x, y
    return None, None


def determine_column_index(x_pos, viewbox_width):
    """
    Determine colour index from X position (4 vertical columns).
    """
    if x_pos is None:
        return 0  # unknown

    band_width = viewbox_width / 4.0

    if x_pos < band_width:
        return 1  # purple (left)
    elif x_pos < band_width * 2:
        return 2  # mint
    elif x_pos < band_width * 3:
        return 3  # coral
    else:
        return 4  # blue (right)


def is_above_pink_line(y_pos, viewbox_height):
    """Return True if segment is above the pink divider line."""
    if y_pos is None:
        return False
    # Move threshold closer to the top so only score/header crabs are _0
    pink_threshold = viewbox_height * 0.10
    return y_pos < pink_threshold


def process_svg_file(svg_path, out_path):
    """Process gnw_cgrab.svg and add colour indices to segment titles."""
    print(f"\nProcessing: {svg_path.name}")

    tree = ET.parse(svg_path)
    root = tree.getroot()

    # viewBox="0 0 2084 3696"
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
        if x_pos is None:
            print(f"  Warning: no position for {current_title}")
            continue

        if is_above_pink_line(y_pos, viewbox_height):
            color_idx = 0
            reason = "above pink line"
        else:
            color_idx = determine_column_index(x_pos, viewbox_width)
            if color_idx == 0:
                print(f"  Warning: could not assign colour for {current_title}")
                continue
            reason = None

        new_title = f"{current_title}_{color_idx}"
        title_elem.text = new_title

        color_names = {0: "none", 1: "purple", 2: "mint", 3: "coral", 4: "blue"}
        extra = f" ({reason})" if reason else ""
        print(
            f"  {current_title} (x={x_pos:.1f}, y={y_pos:.1f}) "
            f"-> {new_title} ({color_names.get(color_idx, 'unknown')}){extra}"
        )
        modified_count += 1

    tree.write(out_path, encoding="UTF-8", xml_declaration=True)

    print(
        f"\nSummary: {modified_count} segments updated, "
        f"{skipped_count} skipped (already had index)"
    )


def main():
    base = Path(__file__).parent / "rom" / "gnw_cgrab"
    in_svg = base / "gnw_cgrab.svg"
    out_svg = base / "gnw_cgrab_coloured.svg"
    background = base / "Background2.png"
    frame = base / "Frame2.png"
    combined_background = base / "Background2_with_frame.png"

    if not in_svg.exists():
        print(f"ERROR: Input SVG not found: {in_svg}")
        return

    process_svg_file(in_svg, out_svg)
    print(f"\nWrote coloured SVG with indices to: {out_svg}")

    # Combine Background2.png with Frame2.png to embed the pink divider line
    if background.exists() and frame.exists():
        try:
            bg_img = Image.open(background).convert("RGBA")
            frame_img = Image.open(frame).convert("RGBA")

            if frame_img.size != bg_img.size:
                frame_img = frame_img.resize(bg_img.size, resample=Image.BILINEAR)

            combined = bg_img.copy()
            combined.alpha_composite(frame_img)
            combined.save(combined_background)
            print(f"Combined background+frame saved to: {combined_background}")
        except Exception as e:
            print(f"ERROR combining background and frame: {e}")
    else:
        if not background.exists():
            print(f"WARNING: Background image not found: {background}")
        if not frame.exists():
            print(f"WARNING: Frame image not found: {frame}")


if __name__ == "__main__":
    main()