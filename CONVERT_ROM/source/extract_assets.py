"""
Extract and merge Game & Watch ROM assets from artwork and roms folders.

This script:
1. Scans rom/artwork and rom/roms folders for zip files
2. Extracts matching pairs from both folders into rom/<name>
3. Extracts ROM-only files (without artwork) into rom/<name>
4. Merges files from both artwork and roms into the same destination folder
"""

import os
import zipfile
from pathlib import Path
from typing import Dict, List, Tuple

# ROM files that don't have matching artwork files
ROMS_ONLY_FILES = {
    "gnw_helmeto",
    "gnw_judgeo",
    "gnw_mariocmta"
}


def get_zip_files(folder_path: Path) -> Dict[str, Path]:
    """
    Get all zip files in a folder.
    
    Args:
        folder_path: Path to the folder to scan
        
    Returns:
        Dictionary mapping filename (without .zip) to full path
    """
    if not folder_path.exists():
        print(f"Warning: Folder does not exist: {folder_path}")
        return {}
    
    zip_files = {}
    for file in folder_path.glob("*.zip"):
        # Get filename without .zip extension
        name = file.stem
        zip_files[name] = file
    
    return zip_files


def find_matching_pairs(artwork_zips: Dict[str, Path], 
                       roms_zips: Dict[str, Path]) -> Tuple[List[str], List[str], List[str]]:
    """
    Find matching and non-matching zip files between artwork and roms folders.
    
    Args:
        artwork_zips: Dictionary of artwork zip files
        roms_zips: Dictionary of roms zip files
        
    Returns:
        Tuple of (matching_names, artwork_only, roms_only)
    """
    artwork_names = set(artwork_zips.keys())
    roms_names = set(roms_zips.keys())
    
    matching = sorted(artwork_names & roms_names)
    artwork_only = sorted(artwork_names - roms_names)
    roms_only = sorted(roms_names - artwork_names)
    
    return matching, artwork_only, roms_only


def extract_and_merge(name: str, 
                     artwork_zip: Path, 
                     roms_zip: Path, 
                     destination_base: Path,
                     console_folder: Path) -> bool:
    """
    Extract both zip files and merge their contents into a single folder.
    
    Args:
        name: Name of the game (used for destination folder)
        artwork_zip: Path to artwork zip file
        roms_zip: Path to roms zip file
        destination_base: Base path for extraction (typically rom/)
        
    Returns:
        True if successful, False otherwise
    """
    destination = destination_base / name
    
    try:
        # Create destination folder if it doesn't exist
        destination.mkdir(parents=True, exist_ok=True)
        
        # Extract artwork zip
        with zipfile.ZipFile(artwork_zip, 'r') as zip_ref:
            zip_ref.extractall(destination)
            artwork_files = len(zip_ref.namelist())
        
        # Extract roms zip (may overwrite files from artwork)
        with zipfile.ZipFile(roms_zip, 'r') as zip_ref:
            zip_ref.extractall(destination)
            roms_files = len(zip_ref.namelist())

        console_suffix = copy_console_image_and_suffix(name, console_folder, destination)
        print(
            f"  ✓ {name:<25} → {artwork_files} artwork + {roms_files} rom files" 
            f"{console_suffix}"
        )
        
        return True
        
    except zipfile.BadZipFile as e:
        print(f"  ✗ {name:<25} → Error: Invalid zip file - {e}")
        return False
    except Exception as e:
        print(f"  ✗ {name:<25} → Error: {e}")
        return False


def extract_rom_only(name: str, 
                     roms_zip: Path, 
                     destination_base: Path,
                     console_folder: Path) -> bool:
    """
    Extract a ROM zip file that has no matching artwork.
    
    Args:
        name: Name of the game (used for destination folder)
        roms_zip: Path to roms zip file
        destination_base: Base path for extraction (typically rom/)
        
    Returns:
        True if successful, False otherwise
    """
    destination = destination_base / name
    
    try:
        # Create destination folder if it doesn't exist
        destination.mkdir(parents=True, exist_ok=True)
        
        # Extract roms zip
        with zipfile.ZipFile(roms_zip, 'r') as zip_ref:
            zip_ref.extractall(destination)
            roms_files = len(zip_ref.namelist())

        console_suffix = copy_console_image_and_suffix(name, console_folder, destination)
        print(
            f"  ✓ {name:<25} → {roms_files} rom files (no artwork)" 
            f"{console_suffix}"
        )
        
        return True
        
    except zipfile.BadZipFile as e:
        print(f"  ✗ {name:<25} → Error: Invalid zip file - {e}")
        return False
    except Exception as e:
        print(f"  ✗ {name:<25} → Error: {e}")
        return False


def copy_console_image_and_suffix(name: str, console_folder: Path, destination: Path) -> str:
    """Copy console PNG into destination and return info suffix.

    Returns " + 1 console file" if the console image was copied,
    otherwise " (no console file)". Errors are non-fatal.
    """
    console_src = console_folder / f"{name}.png"
    if not console_src.exists():
        return " (no console file)"

    try:
        console_dst = destination / f"{name}.png"
        with console_src.open("rb") as src_f, console_dst.open("wb") as dst_f:
            dst_f.write(src_f.read())
        return " + 1 console file"
    except Exception as e:
        print(f"  ! Warning: could not copy console image for {name}: {e}")
        return " (no console file)"


def extract_assets():
    """Main execution function."""
    # Set up paths
    script_dir = Path(__file__).parent.parent
    artwork_folder = script_dir / "rom" / "artwork"
    roms_folder = script_dir / "rom" / "roms"
    console_folder = script_dir / "rom" / "console"
    destination_base = script_dir / "rom" / "decompress"
    
    print("=" * 70)
    print("Game & Watch Asset Extractor")
    print("=" * 70)
    
    # Get all zip files from both folders
    print("\nScanning folders...")
    artwork_zips = get_zip_files(artwork_folder)
    roms_zips = get_zip_files(roms_folder)
    
    print(f"  Artwork folder: {len(artwork_zips)} zip files")
    print(f"  Roms folder: {len(roms_zips)} zip files")
    
    # Find matching pairs and mismatches
    matching, artwork_only, roms_only = find_matching_pairs(artwork_zips, roms_zips)
    
    # Separate expected ROM-only files from unexpected mismatches
    expected_roms_only = [name for name in roms_only if name in ROMS_ONLY_FILES]
    unexpected_roms_only = [name for name in roms_only if name not in ROMS_ONLY_FILES]
    
    # Report errors if any unexpected mismatches found
    has_errors = False
    
    if artwork_only:
        has_errors = True
        print("\n⚠ ERROR: Files in artwork folder without matching roms:")
        for name in artwork_only:
            print(f"    - {name}.zip")
    
    if unexpected_roms_only:
        has_errors = True
        print("\n⚠ ERROR: Files in roms folder without matching artwork:")
        for name in unexpected_roms_only:
            print(f"    - {name}.zip")
    
    if has_errors:
        print("\n" + "=" * 70)
        print("Please ensure all zip files have matching pairs in both folders.")
        print("=" * 70)
        return
    
    # Process matching pairs
    total_to_process = len(matching) + len(expected_roms_only)
    
    if total_to_process == 0:
        print("\n⚠ No zip files found to process.")
        return
    
    print(f"\n✓ Found {len(matching)} matching pairs")
    if expected_roms_only:
        print(f"✓ Found {len(expected_roms_only)} ROM-only files (no artwork)")
    
    print("\nExtracting assets:")
    
    success_count = 0
    fail_count = 0
    
    # Process matching pairs
    for name in matching:
        success = extract_and_merge(
            name,
            artwork_zips[name],
            roms_zips[name],
            destination_base,
            console_folder
        )
        
        if success:
            success_count += 1
        else:
            fail_count += 1
    
    # Process ROM-only files
    for name in expected_roms_only:
        success = extract_rom_only(
            name,
            roms_zips[name],
            destination_base,
            console_folder
        )
        
        if success:
            success_count += 1
        else:
            fail_count += 1
    
    # Summary
    print("\n" + "=" * 70)
    print("Summary:")
    print(f"  ✓ Successfully extracted: {success_count}")
    if fail_count > 0:
        print(f"  ✗ Failed: {fail_count}")
    print("=" * 70)
    print("\n")


if __name__ == "__main__":
    extract_assets()
