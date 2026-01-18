import subprocess
import xml.etree.ElementTree as ET
import os
import re
import urllib.request
import urllib.error

# --- RESTORED CONFIGURATION ---
try:
    from external_apps import MAME_PATH
except ImportError:
    MAME_PATH = r"C:\MAME"

MAME_EXE = os.path.join(MAME_PATH, "mame.exe")
TEMP_XML = os.path.join(os.path.dirname(__file__), "mame_raw.xml")
OUTPUT_MD = os.path.join(os.path.dirname(__file__), "EXTRACTED_LIST.md")
MAME_LISTXML_STDERR = os.path.join(os.path.dirname(__file__), "mame_listxml_stderr.txt")
KEEP_INTERMEDIATE = False

# Upstream MAME driver source contains explicit model codes in comment headers,
# e.g. "Nintendo Game & Watch: Chef (model FP-24)".
HH_SM510_URL = "https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_sm510.cpp"
HH_SM510_CACHE = os.path.join(os.path.dirname(__file__), "tmp", "mame_hh_sm510.cpp")


def _ensure_parent_dir(file_path: str) -> None:
    parent = os.path.dirname(file_path)
    if parent:
        os.makedirs(parent, exist_ok=True)


def _read_text(path: str) -> str:
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def _download_text(url: str) -> str:
    req = urllib.request.Request(url, headers={"User-Agent": "yokoi-extractor/1.0"})
    with urllib.request.urlopen(req, timeout=30) as resp:
        raw = resp.read()
    return raw.decode("utf-8", errors="replace")


def _normalize_model_token(value: str) -> str:
    # Normalize model/ROM ID tokens so that (model FP-24) matches ROM_LOAD("fp-24").
    token = (value or "").strip()
    token = token.replace("_", "-").replace(" ", "")
    return token.lower()


def _canonical_model(value: str) -> str:
    token = (value or "").strip()
    token = re.sub(r"\s+", " ", token)
    return token.upper()


def load_models_from_hh_sm510(url: str = HH_SM510_URL, cache_path: str = HH_SM510_CACHE) -> dict[str, str]:
    """Build mapping from MAME set shortname -> Nintendo-style model code.

    Strategy:
    - Parse model codes from comment headers: '(model XX-YY)'
    - Parse ROM_START blocks and their ROM_LOAD labels
    - If a ROM_LOAD base token matches a known model code, assign it to that set
    """

    source: str | None = None
    if os.path.exists(cache_path) and os.path.getsize(cache_path) > 0:
        source = _read_text(cache_path)
    else:
        try:
            source = _download_text(url)
            _ensure_parent_dir(cache_path)
            with open(cache_path, "w", encoding="utf-8", errors="replace") as f:
                f.write(source)
        except (urllib.error.URLError, TimeoutError, OSError) as e:
            print(f"WARNING: Could not download MAME source for model codes: {e}")
            return {}

    # 1) Collect known model codes from comment headers.
    model_norm_to_canon: dict[str, str] = {}
    model_header_re = re.compile(r"\(model\s+([^)]+)\)")
    for line in source.splitlines():
        for raw_model in model_header_re.findall(line):
            canon = _canonical_model(raw_model)
            norm = _normalize_model_token(raw_model)
            if norm:
                model_norm_to_canon.setdefault(norm, canon)

    if not model_norm_to_canon:
        return {}

    # 2) Parse ROM_START blocks and match ROM_LOAD labels against model codes.
    set_to_model: dict[str, str] = {}
    rom_start_re = re.compile(r"^\s*ROM_START\(\s*([a-z0-9_]+)\s*\)")
    rom_end_re = re.compile(r"^\s*ROM_END\b")
    rom_load_re = re.compile(r"\bROM_LOAD\w*\(\s*\"([^\"]+)\"")

    current_set: str | None = None
    current_rom_labels: list[str] = []

    model_like_re = re.compile(r"^([a-z]{1,3})-?(\d{2,3})([a-z])?$", re.IGNORECASE)

    def _finalize_current() -> None:
        nonlocal current_set, current_rom_labels
        if not current_set:
            return

        for label in current_rom_labels:
            base = (label.split(".")[0] if label else "").strip()
            if not base:
                continue

            # Some ROM labels include extra suffixes after '_' (e.g. "bx-301_744").
            # Try the full token and the prefix before '_' as candidates.
            candidates = [base]
            if "_" in base:
                candidates.append(base.split("_", 1)[0])

            for cand in candidates:
                cand_clean = cand.replace("_", "-").strip()
                cand_norm = _normalize_model_token(cand_clean)
                if not cand_norm:
                    continue

                model = model_norm_to_canon.get(cand_norm)
                if model:
                    set_to_model[current_set] = model
                    return

                # Fallback: if the ROM label itself looks like a model code (common for G&W),
                # use it even if it wasn't present in a '(model ...)' header.
                m_like = model_like_re.match(cand_clean)
                if m_like:
                    prefix, digits, suffix = m_like.group(1), m_like.group(2), m_like.group(3)
                    set_to_model[current_set] = _format_model_code(prefix, digits, suffix)
                    return

        current_set = None
        current_rom_labels = []

    for line in source.splitlines():
        m_start = rom_start_re.match(line)
        if m_start:
            _finalize_current()
            current_set = m_start.group(1)
            current_rom_labels = []
            continue

        if current_set:
            m_load = rom_load_re.search(line)
            if m_load:
                current_rom_labels.append(m_load.group(1))
                continue

            if rom_end_re.match(line):
                _finalize_current()

    _finalize_current()
    return set_to_model


def _format_model_code(prefix: str, digits: str, suffix: str | None) -> str:
    # Example outputs: AC-01, BF-803, CM-72A
    code = f"{prefix.upper()}-{digits}"
    if suffix:
        code += suffix.upper()
    return code


def detect_model_from_mame_roms(machine_elem: ET.Element) -> str:
    """Best-effort model code extraction using MAME ROM filenames.

    Many MAME handheld sets embed catalog/model IDs at the start of ROM names
    (e.g. ac01_*, bf803_*, cm72a_*). If not present, returns "(N/A)".
    """
    model_from_rom_re = re.compile(r"^([a-z]{1,3})(\d{2,3})([a-z])?(?:[_\-.]|$)", re.IGNORECASE)

    for rom in machine_elem.iter('rom'):
        name = (rom.get('name', '') or '').strip()
        if not name:
            continue
        m = model_from_rom_re.match(name)
        if not m:
            continue
        prefix, digits, suffix = m.group(1), m.group(2), m.group(3)
        # Filter out obvious false positives: model codes are typically 2 letters.
        # If MAME includes other prefixes, we still allow 3-letter codes.
        if len(prefix) < 2:
            continue
        return _format_model_code(prefix, digits, suffix)

    return "(N/A)"

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

    # Optional: augment model codes using upstream MAME driver source.
    # If download fails, we fall back to ROM-name heuristics.
    models_by_set = load_models_from_hh_sm510()

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
                    
                    # Model Detection:
                    # 1) Prefer MAME driver source mapping (authoritative for G&W model codes)
                    # 2) Fall back to ROM filename heuristics
                    set_name = (elem.get('name') or '').strip()
                    model = models_by_set.get(set_name) if set_name else None
                    if not model:
                        model = detect_model_from_mame_roms(elem)

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
    if found_count > 1 and not KEEP_INTERMEDIATE:
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
