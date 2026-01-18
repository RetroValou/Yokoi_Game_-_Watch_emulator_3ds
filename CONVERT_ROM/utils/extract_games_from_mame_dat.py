import os
import re
import ssl
import subprocess
import html as _html
import urllib.error
import urllib.request
import xml.etree.ElementTree as ET


PROGETTOSNAPS_MAME_DATS_PACK_URL_TEMPLATE = "https://www.progettosnaps.net/dats/MAME/packs/MAME_Dats_{pack}.7z"
LUCKLESSHEAVEN_GNW_RELEASE_ORDER_URL = "https://www.lucklessheaven.com/gameandwatch-release-order"

SCRIPT_DIR = os.path.dirname(__file__)
CONVERT_ROM_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))

# Cache downloads/extracted dats under CONVERT_ROM/tmp
DEFAULT_CACHE_DIR = os.path.join(CONVERT_ROM_DIR, "tmp")

# Filter just the handheld driver you care about (matches the example)
DEFAULT_SOURCEFILE_FILTER = "handheld/hh_sm510.cpp"

# Hardcoded version to use (edit this when a new MAME comes out)
MAME_DAT_VERSION = "0.284"

# This environment doesn't have working CA certs, so always download with
# TLS verification disabled.
ALWAYS_INSECURE_TLS = True

# When enabled, Nintendo entries get their Release Date replaced with exact dates
# from Luckless Heaven's Game & Watch release order page.
CROSS_REFERENCE_NINTENDO_RELEASE_DATES = True

# Output stays in CONVERT_ROM/utils
DEFAULT_OUTPUT_MD = os.path.join(SCRIPT_DIR, "EXTRACTED_LIST_FROM_MAMEDAT.md")

# Manual overrides for cases where the model code can't be reliably derived.
# Key is the MAME machine shortname (the zip name without extension).
# Example: if you know the real model code for a set, add it here.
SET_MODEL_OVERRIDES: dict[str, str] = {
    # Egg uses the same ROM/model token as some variants in MAME.dat, but the real
    # Game & Watch model number is EG-26.
    "gnw_egg": "EG-26",
    "tigarden": "TG-18",
    "trclchick": "CC-38V",
    "trdivadv": "DA-37", 
    "trsgkeep": "SK-10",
    "trshutvoy": "MG-8",
    "trspacadv": "SA-12",
    "trspacmis": "SM-11",
    "trspider": "SG-21",
    "trsrescue": "MG-9",
    "trthuball": "FR-23",
    "trtreisl": "TI-31"
}


def _ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def _download_file(url: str, dest_path: str) -> None:
    context = ssl._create_unverified_context() if ALWAYS_INSECURE_TLS else None
    req = urllib.request.Request(url, headers={"User-Agent": "yokoi-mame-dat/1.0"})
    with urllib.request.urlopen(req, timeout=120, context=context) as resp:
        _ensure_dir(os.path.dirname(dest_path))
        with open(dest_path, "wb") as f:
            while True:
                chunk = resp.read(1024 * 1024)
                if not chunk:
                    break
                f.write(chunk)


def _download_text(url: str) -> str:
    context = ssl._create_unverified_context() if ALWAYS_INSECURE_TLS else None
    req = urllib.request.Request(url, headers={"User-Agent": "yokoi-mame-dat/1.0"})
    with urllib.request.urlopen(req, timeout=60, context=context) as resp:
        raw = resp.read()
    return raw.decode("utf-8", errors="replace")


def _html_to_text(html: str) -> str:
    # Make common break tags become newlines first, then strip tags.
    s = re.sub(r"(?is)<(script|style)\b.*?>.*?</\1>", "", html)
    s = re.sub(r"(?i)<br\s*/?>", "\n", s)
    s = re.sub(r"(?i)</(p|div|h1|h2|h3|h4|li|tr|table)>", "\n", s)
    s = re.sub(r"<[^>]+>", "", s)
    s = _html.unescape(s)
    # Normalise whitespace but keep newlines meaningful.
    s = re.sub(r"\r\n?", "\n", s)
    s = re.sub(r"[ \t\f\v]+", " ", s)
    s = re.sub(r"\n{3,}", "\n\n", s)
    return s


def load_gnw_release_dates(cache_dir: str) -> dict[str, str]:
    """Return mapping of model code (e.g., 'AC-01') to a release date string.

    Prefers JP date, then US, then EU. Date strings are kept in the page format
    (e.g., '28 Apr 1980', 'Apr 2010').
    """

    cache_path = os.path.join(cache_dir, "lucklessheaven_gnw_release_order.html")
    if not os.path.exists(cache_path) or os.path.getsize(cache_path) == 0:
        html_text = _download_text(LUCKLESSHEAVEN_GNW_RELEASE_ORDER_URL)
        _ensure_dir(os.path.dirname(cache_path))
        with open(cache_path, "w", encoding="utf-8") as f:
            f.write(html_text)
    else:
        with open(cache_path, "r", encoding="utf-8", errors="replace") as f:
            html_text = f.read()

    text = _html_to_text(html_text)

    # Model codes on the page include classic G&W and newer releases.
    model_re = re.compile(r"\b([A-Z]{1,4}-\d{2,4}[A-Z]?)\b")

    # Dates appear as '28 Apr 1980' or 'Apr 2010'.
    date_re = re.compile(r"\b(\d{1,2}\s+[A-Za-z]{3}\s+\d{4}|[A-Za-z]{3}\s+\d{4})\b")

    matches = list(model_re.finditer(text))
    out: dict[str, str] = {}

    for i, m in enumerate(matches):
        model = m.group(1).upper()
        start = m.start()
        end = matches[i + 1].start() if i + 1 < len(matches) else min(len(text), start + 2000)
        block = text[start:end]

        # Only consider blocks that have a release date section.
        rel_idx = block.lower().find("release date")
        if rel_idx == -1:
            continue
        rel_block = block[rel_idx:]

        def pick_region(region: str) -> str | None:
            # Find 'JP:' then take the first date after it.
            idx = rel_block.find(region + ":")
            if idx == -1:
                return None
            sub = rel_block[idx : idx + 200]
            dm = date_re.search(sub)
            return dm.group(1) if dm else None

        date = pick_region("JP") or pick_region("US") or pick_region("EU")
        if date:
            out[model] = date

    # Luckless Heaven appears to have a typo for Oil Panic.
    # The real model code is OP-51, but the page may list OL-51.
    if "OL-51" in out and "OP-51" not in out:
        out["OP-51"] = out["OL-51"]

    return out


_MONTHS = {
    "jan": "01",
    "feb": "02",
    "mar": "03",
    "apr": "04",
    "may": "05",
    "jun": "06",
    "jul": "07",
    "aug": "08",
    "sep": "09",
    "oct": "10",
    "nov": "11",
    "dec": "12",
}


def format_release_date(date_str: str) -> str:
    """Format dates as YYYY.MM.DD.

    Supported inputs:
    - '27 Apr 1981' -> '1981.04.27'
    - 'Apr 2010' -> '2010.04.01'
    - '1989' -> '1989.01.01'
    If unknown/ambiguous ('19??', '199?'), returns original.
    """

    s = (date_str or "").strip()
    if not s or s.lower() == "unknown":
        return "Unknown"

    # Keep ambiguous year strings as-is.
    if re.fullmatch(r"\d\d\?\?", s) or re.fullmatch(r"\d\d\d\?", s):
        return s

    m = re.fullmatch(r"(\d{1,2})\s+([A-Za-z]{3})\s+(\d{4})", s)
    if m:
        dd = f"{int(m.group(1)):02d}"
        mm = _MONTHS.get(m.group(2).lower(), "00")
        yyyy = m.group(3)
        return f"{yyyy}.{mm}.{dd}"

    m = re.fullmatch(r"([A-Za-z]{3})\s+(\d{4})", s)
    if m:
        mm = _MONTHS.get(m.group(1).lower(), "00")
        yyyy = m.group(2)
        return f"{yyyy}.{mm}.01"

    m = re.fullmatch(r"\d{4}", s)
    if m:
        return f"{s}.01.01"

    return s


def version_to_pack_id(version: str) -> str:
    # ProgettoSnaps pack names use the minor version number only:
    # 0.284 -> 284, 0.283 -> 283
    if not version or "." not in version:
        raise ValueError(f"Invalid version: {version!r}")
    return version.split(".", 1)[1]


def build_pack_url(version: str) -> str:
    return PROGETTOSNAPS_MAME_DATS_PACK_URL_TEMPLATE.format(pack=version_to_pack_id(version))


def _extract_7z(archive_path: str, out_dir: str) -> None:
    """Extract a .7z archive.

    Tries (in order):
    - py7zr (pure Python)
    - 7z (system-installed 7-Zip)
    """

    # 1) py7zr (preferred)
    try:
        import py7zr  # type: ignore

        with py7zr.SevenZipFile(archive_path, mode="r") as z:
            z.extractall(path=out_dir)
        return
    except ModuleNotFoundError:
        pass

    # 2) 7z.exe / 7z
    cmd = shutil_which("7z")
    if not cmd:
        # Common default path on Windows
        candidate = r"C:\Program Files\7-Zip\7z.exe"
        if os.path.exists(candidate):
            cmd = candidate

    if not cmd:
        raise RuntimeError(
            "No .7z extractor found. Install 7-Zip (7z.exe) or install Python package 'py7zr'."
        )

    _ensure_dir(out_dir)
    # 7z x archive -oOUTDIR -y
    proc = subprocess.run([cmd, "x", archive_path, f"-o{out_dir}", "-y"], capture_output=True, text=True)
    if proc.returncode != 0:
        raise RuntimeError(f"7z failed ({proc.returncode}):\n{proc.stdout}\n{proc.stderr}")


def select_best_dat_file(unpack_dir: str, version: str) -> str | None:
    candidates: list[str] = []
    for root, _, files in os.walk(unpack_dir):
        for fn in files:
            if fn.lower().endswith(".dat"):
                candidates.append(os.path.join(root, fn))

    if not candidates:
        return None

    version_l = version.lower()

    def score(path: str) -> tuple[int, str]:
        base = os.path.basename(path).lower()
        if base == "mame.dat":
            s = 100
        elif base == f"mame {version_l}.dat":
            s = 95
        elif base.startswith("mame ") and version_l in base:
            s = 90
        elif "mame" in base and version_l in base:
            s = 80
        elif base.startswith("mame "):
            s = 70
        elif "mame" in base:
            s = 60
        elif "arcade" in base:
            s = 10
        else:
            s = 0
        return (s, path)

    # max by score, stable by path
    return max(candidates, key=score)


def shutil_which(cmd: str) -> str | None:
    # minimal replacement for shutil.which (avoid new import churn)
    paths = os.environ.get("PATH", "").split(os.pathsep)
    exts = [""]
    if os.name == "nt":
        exts = [".exe", ".cmd", ".bat", ""]
    for p in paths:
        p = p.strip('"')
        if not p:
            continue
        for ext in exts:
            candidate = os.path.join(p, cmd + ext)
            if os.path.isfile(candidate):
                return candidate
    return None


def _format_model_code(prefix: str, digits: str, suffix: str | None) -> str:
    # Example outputs: AC-01, BF-803, CM-72A
    code = f"{prefix.upper()}-{digits}"
    if suffix:
        code += suffix.upper()
    return code


def normalize_model_from_rom_name(rom_name: str) -> str:
    """Derive the model code from the first ROM name."""

    token = (rom_name or "").strip()
    if not token:
        return "(N/A)"

    # Drop common artwork and non-program files
    lower = token.lower()
    if lower.endswith((".svg", ".png", ".jpg", ".jpeg")):
        return "(N/A)"

    # Drop extension (only '.' denotes an extension separator)
    base = token.rsplit(".", 1)[0].strip()

    # Some ROM names use an underscore to denote a variant/suffix (e.g. bx-301_744).
    # Do NOT treat underscores as extensions. Only drop the suffix when the prefix
    # already looks like a real model code and the suffix is numeric.
    if "_" in base:
        prefix, suffix = base.split("_", 1)
        prefix = prefix.strip()
        suffix = suffix.strip()
        if re.fullmatch(r"\d+", suffix) and (
            "-" in prefix or re.match(r"^[a-z]{1,4}-?\d{2,4}[a-z]?$", prefix, re.IGNORECASE)
        ):
            base = prefix

    base = base.strip()

    # Normalize compact IDs like bf803 -> BF-803, ac01 -> AC-01
    m = re.match(r"^([a-z]{1,3})-?(\d{2,3})([a-z])?$", base, re.IGNORECASE)
    if m and len(m.group(1)) >= 2:
        return _format_model_code(m.group(1), m.group(2), m.group(3))

    # If it already contains a dash, keep it
    if "-" in base:
        return base.upper()

    return base.upper()


def pick_cpu_from_device_refs(machine_elem: ET.Element) -> str:
    # Example: <device_ref name="sm510"/>
    for dev in machine_elem.findall("device_ref"):
        name = (dev.get("name") or "").strip().lower()
        if not name:
            continue

        # Prefer Sharp SM5xx family
        if name in {"sm5a", "sm510", "sm511", "sm512", "sm530"}:
            return name.upper()

    # Fallback: first device_ref that looks like sm* but isn't above list
    for dev in machine_elem.findall("device_ref"):
        name = (dev.get("name") or "").strip().lower()
        if name.startswith("sm"):
            return name.upper()

    return "Unknown"


def iter_hh_sm510_machines(dat_path: str, sourcefile_filter: str) -> list[dict[str, str]]:
    """Parse mame.dat and extract machines from a specific MAME sourcefile."""

    extracted: list[dict[str, str]] = []
    sourcefile_filter_norm = (sourcefile_filter or "").replace("\\", "/").lower()

    context = ET.iterparse(dat_path, events=("end",))
    for _, elem in context:
        if elem.tag != "machine":
            continue

        sourcefile = (elem.get("sourcefile") or "").replace("\\", "/").lower()
        if sourcefile_filter_norm and sourcefile_filter_norm not in sourcefile:
            elem.clear()
            continue

        set_name = (elem.get("name") or "").strip()
        if not set_name:
            elem.clear()
            continue

        cloneof = (elem.get("cloneof") or "").strip()
        romof = (elem.get("romof") or "").strip()

        title = (elem.findtext("description") or "Unknown").strip() or "Unknown"
        year = (elem.findtext("year") or "Unknown").strip() or "Unknown"
        manufacturer = (elem.findtext("manufacturer") or "Unknown").strip() or "Unknown"

        # Model: allow explicit overrides, otherwise derive from first ROM name (excluding artwork)
        model = (SET_MODEL_OVERRIDES.get(set_name) or "").strip().upper()
        if not model:
            model = "(N/A)"
            for rom in elem.findall("rom"):
                rn = (rom.get("name") or "").strip()
                if not rn:
                    continue
                candidate = normalize_model_from_rom_name(rn)
                if candidate != "(N/A)":
                    model = candidate
                    break

        cpu = pick_cpu_from_device_refs(elem)

        extracted.append(
            {
                "manufacturer": manufacturer,
                "set_name": set_name,
                "cloneof": cloneof,
                "romof": romof,
                "zip_name": f"{set_name}.zip",
                "title": title,
                "year": year,
                "release_date": year,
                "cpu": cpu,
                "model_base": model,
                "model": model,
            }
        )

        elem.clear()

    return extracted


def write_markdown(entries: list[dict[str, str]], output_md: str) -> None:
    groups: dict[str, list[dict[str, str]]] = {}
    for e in entries:
        groups.setdefault(e["manufacturer"], []).append(e)

    lines: list[str] = []
    for manufacturer in sorted(groups.keys(), key=lambda s: s.lower()):
        items = groups[manufacturer]
        items.sort(key=lambda x: x["zip_name"].lower())

        lines.append(f"## {manufacturer}\n\n")
        lines.append("| No. | Model   | Filename            | Game Title                                                   | Release Date | CPU |\n")
        lines.append("|-----|---------|---------------------|--------------------------------------------------------------|--------------|-----|\n")

        for i, e in enumerate(items, start=1):
            release_date = format_release_date(e.get("release_date", e.get("year", "Unknown")))
            lines.append(
                f"| {i:02}  | {e.get('model_display', e['model']):<7} | {e['zip_name']:<19} | {e['title']:<60} | {release_date:<12} | {e['cpu']:<4} |\n"
            )
        lines.append("\n")

    with open(output_md, "w", encoding="utf-8") as f:
        f.writelines(lines)


def disambiguate_duplicate_models(entries: list[dict[str, str]]) -> None:
    """If multiple entries share the same base model, make the model column unique.

    Rule:
    - Determine the "original" by MAME's rom parenting:
      - Prefer an entry with empty romof
      - Else prefer an entry referenced as romof by another
      - Else fall back to lexicographically smallest set name
    - For non-original entries, append "-<setname>" to the displayed model.
    """

    by_model: dict[str, list[dict[str, str]]] = {}
    for e in entries:
        base = (e.get("model_base") or e.get("model") or "").strip()
        if not base or base == "(N/A)":
            continue
        by_model.setdefault(base, []).append(e)

    for base, group in by_model.items():
        if len(group) <= 1:
            continue

        # Determine which set is the ROM parent, if any.
        referenced_parents = {((e.get("romof") or "").strip()) for e in group if (e.get("romof") or "").strip()}

        def orig_score(e: dict[str, str]) -> tuple[int, str]:
            romof = (e.get("romof") or "").strip()
            set_name = (e.get("set_name") or "").strip()
            # Higher is better.
            s = 0
            if not romof:
                s += 100
            if set_name and set_name in referenced_parents:
                s += 50
            return (s, set_name.lower())

        original = max(group, key=orig_score)

        for e in group:
            set_name = (e.get("set_name") or "").strip()
            if e is original or not set_name:
                e["model_display"] = base
            else:
                e["model_display"] = f"{base}-{set_name}"


def main() -> int:
    version = MAME_DAT_VERSION
    cache_dir = DEFAULT_CACHE_DIR
    sourcefile = DEFAULT_SOURCEFILE_FILTER
    output = DEFAULT_OUTPUT_MD

    _ensure_dir(cache_dir)

    pack_url = build_pack_url(version)
    pack_path = os.path.join(cache_dir, f"mame_dats_{version_to_pack_id(version)}.7z")
    unpack_dir = os.path.join(cache_dir, f"mame_dats_{version_to_pack_id(version)}")

    if not os.path.exists(pack_path) or os.path.getsize(pack_path) == 0:
        print(f"Downloading {pack_url} ...")
        try:
            _download_file(pack_url, pack_path)
        except (urllib.error.URLError, TimeoutError, OSError) as e:
            print(f"ERROR: Failed to download 7z pack: {e}")
            return 2
    else:
        print(f"Using cached 7z pack: {pack_path}")

    dat_path = select_best_dat_file(unpack_dir, version) or ""
    if not dat_path or not os.path.exists(dat_path) or os.path.getsize(dat_path) == 0:
        try:
            print(f"Extracting pack -> {unpack_dir}")
            _extract_7z(pack_path, unpack_dir)
        except Exception as e:
            print(f"ERROR: Failed to extract .7z pack: {e}")
            return 2

        dat_path = select_best_dat_file(unpack_dir, version) or ""
        if not dat_path or not os.path.exists(dat_path) or os.path.getsize(dat_path) == 0:
            print("ERROR: Extracted pack but no suitable .dat file was found")
            return 2
    else:
        print(f"Using cached dat: {dat_path}")

    print(f"Parsing {dat_path} (filter: {sourcefile})...")
    entries = iter_hh_sm510_machines(dat_path, sourcefile)

    if CROSS_REFERENCE_NINTENDO_RELEASE_DATES and entries:
        try:
            release_dates = load_gnw_release_dates(cache_dir)
            if release_dates:
                entries_by_set: dict[str, dict[str, str]] = {
                    (e.get("set_name") or ""): e for e in entries if (e.get("set_name") or "")
                }

                clones_by_parent: dict[str, list[dict[str, str]]] = {}
                for e in entries:
                    parent = (e.get("cloneof") or "").strip()
                    if parent:
                        clones_by_parent.setdefault(parent, []).append(e)

                for e in entries:
                    if e.get("manufacturer") != "Nintendo":
                        continue
                    model = (e.get("model_base") or e.get("model") or "").upper()
                    if model and model in release_dates:
                        e["release_date"] = release_dates[model]

                # If the site is missing alternate set entries (e.g. gnw_helmet vs gnw_helmeto),
                # copy the release date from the parent set when available.
                for e in entries:
                    if e.get("manufacturer") != "Nintendo":
                        continue
                    model = (e.get("model_base") or e.get("model") or "").upper()
                    if model and model in release_dates:
                        continue

                    parent = (e.get("cloneof") or "").strip()
                    if not parent:
                        continue
                    parent_entry = entries_by_set.get(parent)
                    if not parent_entry:
                        continue
                    parent_date = parent_entry.get("release_date")
                    if parent_date:
                        e["release_date"] = parent_date

                # Reverse case: if the *parent* entry is missing from the site
                # but a clone entry has a real date, copy that date back.
                for e in entries:
                    if e.get("manufacturer") != "Nintendo":
                        continue

                    model = (e.get("model_base") or e.get("model") or "").upper()
                    if model and model in release_dates:
                        continue

                    # Only override placeholders like "1981".
                    current = (e.get("release_date") or "").strip()
                    if not re.fullmatch(r"\d{4}", current):
                        continue

                    set_name = (e.get("set_name") or "").strip()
                    if not set_name:
                        continue

                    best = None
                    for child in clones_by_parent.get(set_name, []):
                        child_date = (child.get("release_date") or "").strip()
                        if child_date and not re.fullmatch(r"\d{4}", child_date):
                            best = child_date
                            break
                    if best:
                        e["release_date"] = best
        except Exception as ex:
            print(f"WARNING: Failed to cross-reference Nintendo release dates: {ex}")

    # Make the model column unique when multiple sets share the same ROM/model.
    disambiguate_duplicate_models(entries)

    if not entries:
        print("WARNING: No entries matched your sourcefile filter.")

    write_markdown(entries, output)
    print(f"SUCCESS: Wrote {len(entries)} entries to {output}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
