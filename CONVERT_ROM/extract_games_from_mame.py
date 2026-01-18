import subprocess
import xml.etree.ElementTree as ET
import os
import re

# --- RESTORED CONFIGURATION ---
try:
    from external_apps import MAME_PATH
except ImportError:
    MAME_PATH = r"C:\MAME"

MAME_EXE = os.path.join(MAME_PATH, "mame.exe")
TEMP_XML = os.path.join(os.path.dirname(__file__), "mame_raw.xml")
OUTPUT_MD = os.path.join(os.path.dirname(__file__), "EXTRACTED_LIST.md")
MAME_LISTXML_STDERR = os.path.join(os.path.dirname(__file__), "mame_listxml_stderr.txt")

def get_mame_lcd_list():
    if not os.path.exists(MAME_EXE):
        print(f"ERROR: MAME not found at {MAME_EXE}")
        return

    # Check if we should generate a new file or use your existing one
    if not os.path.exists(TEMP_XML) or os.path.getsize(TEMP_XML) == 0:
        print("Generating 0.284 XML database (350MB+)...")
        try:
            # Portable + memory-safe: stream stdout directly to disk.
            with open(TEMP_XML, "wb") as out_f, open(MAME_LISTXML_STDERR, "wb") as err_f:
                subprocess.run([MAME_EXE, "-listxml"], stdout=out_f, stderr=err_f, check=True)

            # Quick sanity check (helps diagnose cases where MAME wrote non-XML output).
            if not os.path.exists(TEMP_XML) or os.path.getsize(TEMP_XML) == 0:
                print(f"ERROR: MAME produced an empty XML file: {TEMP_XML}")
                print(f"See stderr log: {MAME_LISTXML_STDERR}")
                return
            with open(TEMP_XML, "rb") as chk_f:
                head = chk_f.read(32)
            if b"<?xml" not in head:
                print("WARNING: XML file does not start with an XML header.")
                print(f"The output may be invalid XML. Inspect: {TEMP_XML}")
                print(f"Also check stderr log: {MAME_LISTXML_STDERR}")
        except subprocess.CalledProcessError as e:
            print("ERROR: Failed to generate MAME XML.")
            print(f"See stderr log: {MAME_LISTXML_STDERR}")
            return
    else:
        print(f"Using existing {TEMP_XML} for parsing...")

    groups = {}
    found_total = 0
    # In newer MAME, the <chip name="..."> field is often a descriptive string
    # like "Sharp SM511" rather than a raw id like "sm511".
    target_cpu_re = re.compile(r"(?<![a-z0-9])(sm5a|sm510|sm511)(?![a-z0-9])")

    try:
        # iterparse is mandatory to handle the massive 0.284 dataset
        context = ET.iterparse(TEMP_XML, events=('end',))
        for event, elem in context:
            if elem.tag == 'machine':
                # 0.284 FIX: Recursive search for chips anywhere in the device tree
                found_cpu = None
                for chip in elem.iter('chip'):
                    if (chip.get('type', '') or '').lower() != 'cpu':
                        continue
                    c_name = (chip.get('name', '') or '').lower()
                    m = target_cpu_re.search(c_name)
                    if m:
                        found_cpu = m.group(1).upper()
                        break
                
                if found_cpu:
                    zip_name = f"{elem.get('name')}.zip"
                    title = elem.findtext('description', 'Unknown')
                    year = (
                        (elem.findtext('year') or '').strip()
                        or (elem.get('year') or '').strip()
                        or 'Unknown'
                    )
                    manufacturer = (elem.findtext('manufacturer', 'Unknown') or 'Unknown').strip() or 'Unknown'
                    
                    # Enhanced Model Detection: Search ROM nodes or ZIP prefix
                    model = "(N/A)"
                    # Search all rom tags within this machine
                    rom_node = elem.find('.//rom')
                    if rom_node is not None:
                        r_name = rom_node.get('name', '')
                        if "_" in r_name:
                            model = r_name.split("_")[0].upper()
                    
                    # Fallback for G&W names
                    if model == "(N/A)" and zip_name.startswith("gnw_"):
                        model = zip_name.split("_")[1].replace(".zip", "").upper()

                    groups.setdefault(manufacturer, []).append(
                        {
                            "model": model,
                            "zip_name": zip_name,
                            "title": title,
                            "year": year,
                            "cpu": found_cpu,
                        }
                    )
                    found_total += 1
                
                elem.clear() # Free memory after each system
    except Exception as e:
        print(f"Parsing error: {e}")

    # Build output Markdown grouped by manufacturer.
    lines = []
    for manufacturer in sorted(groups.keys(), key=lambda s: s.lower()):
        entries = groups[manufacturer]
        entries.sort(key=lambda e: e["zip_name"].lower())

        lines.append(f"## {manufacturer}\n\n")
        lines.append("| No. | Model   | Filename            | Game Title                                                   | Release Date | CPU |\n")
        lines.append("|-----|---------|---------------------|--------------------------------------------------------------|--------------|-----|\n")

        for i, e in enumerate(entries, start=1):
            lines.append(
                f"| {i:02}  | {e['model']:<7} | {e['zip_name']:<19} | {e['title']:<60} | {e['year']:<12} | {e['cpu']:<4} |\n"
            )
        lines.append("\n")

    with open(OUTPUT_MD, "w", encoding="utf-8") as f:
        f.writelines(lines)

    found_count = found_total

    # Cleanup large intermediate files if we got a meaningful result.
    if found_count > 1:
        for p in (TEMP_XML, MAME_LISTXML_STDERR):
            try:
                os.remove(p)
            except FileNotFoundError:
                pass
            except OSError as e:
                print(f"WARNING: Could not delete {p}: {e}")

    print(f"SUCCESS! Found {found_count} games. Results saved to {OUTPUT_MD}")

if __name__ == "__main__":
    get_mame_lcd_list()
