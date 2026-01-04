import os
import glob
import shutil
import importlib.util
from pathlib import Path
from PIL import Image, ImageEnhance
import numpy as np
from rectpack import newPacker
from multiprocessing import Pool
import functools
from typing import Iterable

from source import convert_svg as cs
from source import img_manipulation as im
from source.target_profiles import get_target



INKSCAPE_PATH = r"D:\Program Files\Inkscape\bin\inkscape.exe"

destination_file = r"..\source\std\GW_ALL.h"
destination_game_file = r"..\source\std\GW_ROM"
destination_graphique_file = "../gfx/"

reset_img_svg = False  # default; can be overridden via CLI

# These are set by the selected --target profile at runtime.
resolution_up = [0, 0]
resolution_down = [0, 0]
demi_resolution_up = [0, 0]

size_altas_check = []
pad = 1
console_atlas_size = [0, 0]
console_size = [0, 0]

export_dpi = 50
size_scale = 1
background_atlas_sizes = []

# Target-specific texture output.
# - 3DS uses tex3ds pipeline: .png + .t3s -> build generates .t3x and C++ points to .t3x.
# - RGDS/Android uses runtime PNG loading: .png only; .t3s can exist but is intentionally blank.
texture_path_prefix = "romfs:/gfx/"
texture_path_ext = ".t3x"
tex3ds_enabled = True
gw_rom_include_dir = "GW_ROM"

default_alpha_bright = 1.7
default_fond_bright = 1.35
default_rotate = False
default_console = r'.\rom\default.png'


def _load_games_path_for_target(target_name: str) -> dict:
    """Load games_path dict for the given target.

    Rule:
    - <target> -> games_path_<target>.py

    Paths are resolved relative to this script (CONVERT_ROM/).
    """

    script_dir = Path(__file__).resolve().parent
    filename = f"games_path_{target_name}.py"
    module_path = script_dir / filename
    if not module_path.exists():
        raise RuntimeError(f"{filename} not found at {module_path}")

    spec = importlib.util.spec_from_file_location("games_path_module", str(module_path))
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Unable to load {filename}")

    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)  # type: ignore[assignment]
    loaded = getattr(module, "games_path", None)
    if not isinstance(loaded, dict):
        raise RuntimeError(f"{filename} does not define a 'games_path' dict")

    return loaded

def clean_games_path():
    global games_path
    tmp_gamepath = {}
    for key in games_path:
        new_key = str(key).lower()
        if(new_key not in tmp_gamepath):
            tmp_gamepath[new_key] = games_path[key]
    games_path = tmp_gamepath



def _round_up_to_one_of(value: int, candidates: Iterable[int]) -> int:
    ordered = sorted(int(x) for x in candidates)
    for c in ordered:
        if value <= c:
            return c
    return ordered[-1] if ordered else value


def apply_profile(target_name: str, out_gfx: str | None, out_gw_all: str | None, out_gw_rom_dir: str | None, dpi_override: int | None, scale_override: int | None):
    global destination_file, destination_game_file, destination_graphique_file
    global resolution_up, resolution_down, demi_resolution_up
    global size_altas_check, console_atlas_size, console_size
    global export_dpi, size_scale, background_atlas_sizes
    global texture_path_prefix, texture_path_ext, tex3ds_enabled, gw_rom_include_dir

    profile = get_target(target_name)

    resolution_up = [profile.resolution_up[0], profile.resolution_up[1]]
    resolution_down = [profile.resolution_down[0], profile.resolution_down[1]]
    demi_resolution_up = [profile.demi_resolution_up[0], profile.demi_resolution_up[1]]
    console_size = [profile.console_size[0], profile.console_size[1]]
    console_atlas_size = [profile.console_atlas_size[0], profile.console_atlas_size[1]]
    size_altas_check = list(profile.atlas_sizes)
    background_atlas_sizes = list(profile.background_atlas_sizes)

    export_dpi = int(dpi_override) if dpi_override is not None else int(profile.export_dpi)
    if export_dpi <= 0:
        export_dpi = int(profile.export_dpi)

    size_scale = int(scale_override) if scale_override is not None else int(profile.size_scale)
    if size_scale <= 0:
        size_scale = int(profile.size_scale)

    if out_gw_all is not None:
        destination_file = out_gw_all
    else:
        destination_file = r"..\source\std\GW_ALL.h" if profile.name == "3ds" else r"..\source\std\GW_ALL_rgds.h"

    if out_gw_rom_dir is not None:
        destination_game_file = out_gw_rom_dir
    else:
        destination_game_file = r"..\source\std\GW_ROM" if profile.name == "3ds" else r"..\source\std\GW_ROM_RGDS"

    if profile.name == "3ds":
        texture_path_prefix = "romfs:/gfx/"
        texture_path_ext = ".t3x"
        tex3ds_enabled = True
        gw_rom_include_dir = "GW_ROM"
    else:
        # Android/RGDS runtime loads PNGs directly from assets.
        texture_path_prefix = "gfx/"
        texture_path_ext = ".png"
        tex3ds_enabled = False
        gw_rom_include_dir = "GW_ROM_RGDS"

    if out_gfx is not None:
        destination_graphique_file = out_gfx
    else:
        destination_graphique_file = "../gfx/" if profile.name == "3ds" else "../gfx2x/"

def sort_by_screen(name: str):
    img_screen_sort = []
    for filename in os.listdir("./tmp/img/"+name):
        if(filename[-4:] == '.png'):
            curr_screen = int(filename.split(".")[3])
            while(len(img_screen_sort) <= curr_screen): img_screen_sort.append([])
            img_screen_sort[curr_screen].append(filename)
    return img_screen_sort



def segment_text(all_img, name, color_segment:bool = False):
    result = f"\nconst Segment segment_GW_{name}[] = {{\n	"
    for data in all_img:
        filename, img_r, screen, pos_x, pos_y, size_x, size_y, pos_x_tex, pos_y_tex = data
        seg_x = int(filename.split(".")[0])
        seg_y = int(filename.split(".")[1])
        seg_z = int(filename.split(".")[2].split("_")[0])
        color_index = 0
        if(color_segment): 
            color_index = int(filename.split("_")[1].split(".")[0])
        result += f"{{ {{ {seg_x},{seg_y},{seg_z} }}, {{ {pos_x},{pos_y} }}, {{ {pos_x_tex},{pos_y_tex} }}, {{ {size_x},{size_y} }}, {color_index}, {screen}, false, false, 0 }}, "
    result = result[:-2] + "\n};"
    result += f"  const size_t size_segment_GW_{name} = sizeof(segment_GW_{name})/sizeof(segment_GW_{name}[0]); \n"
    return result


def find_best_parquet(rects: list):
    atlas_size = None
    best_area = float("inf")
    best_packer = None
    
    for w in size_altas_check:
        for h in size_altas_check:
            packer = newPacker(rotation=False)
            for rect in rects: packer.add_rect(*rect)
            packer.add_bin(w, h)
            packer.pack()
            abin = next(iter(packer))
            used_rects = len(list(abin))
            if used_rects == len(rects):  # toutes placées
                area = w * h
                if area < best_area:
                    best_area = area
                    atlas_size = (w, h)
                    best_packer = packer  
    return best_packer, atlas_size


def visual_data_file(name, size_list, background_path_list, rotate = False, mask = False, color_segment = False, two_in_one_screen = False, transform = []):
    """
    """
    img_screen_sort = sort_by_screen(name)
    screen_size = []
    all_img = []
    i = 0
    
    rects = []
    for files_list in img_screen_sort:
        size_update = 0
        new_ratio, x_cut, y_cut = im.ratio_cut(transform, i)
            
        for filename in files_list:
            filepath = os.path.join("./tmp/img/"+name, filename)
            img = Image.open(filepath)
            img_r, pos_x, pos_y, x_size_max, y_size_max = im.convert_to_only_Alpha(img, size_list[i][0], size_list[i][1]
                                                                , respect_ratio= True, rotate_90 = rotate, miror = True
                                                                , new_ratio = new_ratio, cut = [x_cut, y_cut], add_SHARPEN = True)
            if(size_update == 0):
                screen_size.append([x_size_max, y_size_max])
                size_update = 1
                if(mask):
                    img = Image.open(background_path_list[i])
                    img, tmp_x, tmp_y = im.transform_img(img, x_size_max, y_size_max, False, rotate, True)
                    img = ImageEnhance.Brightness(img)
                    img = img.enhance(1.35)
                    data_background = np.array(img)
                    
            if(mask):
                img_r = np.array(img_r)
                img_r[:,:,0:3] = data_background[pos_y:pos_y+img_r.shape[0], pos_x:pos_x+img_r.shape[1], 0:3]
                img_r = Image.fromarray(img_r, mode="RGBA")
            rects.append((img_r.size[0] + pad*2, img_r.size[1] + pad*2, filename))
            all_img.append([filename, img_r, i, x_size_max-pos_x-img_r.size[0], pos_y, img_r.size[0], img_r.size[1]])
        i += 1

    packer, atlas_size = find_best_parquet(rects)            
    im.save_packed_img(packer, all_img, atlas_size, pad, destination_graphique_file, name)        
    with open(destination_graphique_file + 'segment_' + name + '.t3s', "w", encoding="utf-8") as f:
        if tex3ds_enabled:
            if(mask):
                f.write("-f rgba8 -z none\n" + 'segment_' + name + '.png')
            else:
                f.write("-f a8 -z none\n" + 'segment_' + name + '.png')
        else:
            # RGDS/Android does not use tex3ds; keep the file intentionally blank.
            f.write("")
    
    result = f'\nconst std::string path_segment_{name} = "{texture_path_prefix}segment_{name}{texture_path_ext}"; // Visual of segment -> Big unique texture'
    result += segment_text(all_img, name, color_segment)
    
    int_mask = 0
    if(mask): int_mask = 1 # first byte
    if(two_in_one_screen) : int_mask += 2 # second byte
    seg_info = [atlas_size[0], atlas_size[1], 1, int_mask]
    
    for src in screen_size: seg_info = seg_info + src
    seg_info = [str(i) for i in seg_info]
    result += f" const uint16_t segment_info_{name}[] = {{ " + ", ".join(seg_info) + "}; \n"
    return result, screen_size



def background_data_file(name, path_list = [], size_list = [], rotate = False, alpha_bright = 1.7, fond_bright = 1.35, shadow = True):
    i = 0
    atlas_size = [1, 1]
    for size in size_list:
        atlas_size[0] = max(1+atlas_size[0]+1, size[0])
        atlas_size[1] += size[1]+2
   
    for i_a in range(2):
        atlas_size[i_a] = _round_up_to_one_of(atlas_size[i_a], background_atlas_sizes or [atlas_size[i_a]])
        
    result_img = np.zeros((atlas_size[1], atlas_size[0], 4))
    curr_ind_r_img = 1
    info_background = [str(atlas_size[0]), str(atlas_size[1])]
    
    if(len(path_list) > 0 and path_list[0] != ''):
        for path in path_list:
            if(rotate): y_size, x_size  = size_list[i]
            else: x_size, y_size = size_list[i]
                
            img = Image.open(path)

            img, x_size, y_size = im.transform_img(img, x_size, y_size, False, rotate, True)
            data = im.make_alpha(img, fond_bright, alpha_bright)

            result_img[curr_ind_r_img:(data.shape[0]+curr_ind_r_img), 1:data.shape[1]+1, :] = data
            
            info_background.append(str(1)) # pos x
            info_background.append(str(atlas_size[1]-curr_ind_r_img-data.shape[0])) # pos y
            info_background.append(str(data.shape[1])) # size x
            info_background.append(str(data.shape[0])) # size y
            curr_ind_r_img = data.shape[0]+2
            i += 1
        
        img = Image.fromarray(result_img.astype(np.uint8))
        img.save(destination_graphique_file + 'background_' + name + '.png')
        with open(destination_graphique_file + 'background_' + name + '.t3s', "w", encoding="utf-8") as f:
            if tex3ds_enabled:
                f.write("-f RGBA8 -z none\n" + 'background_' + name + '.png')
            else:
                f.write("")
        result = f'\nconst std::string path_background_{name} = "{texture_path_prefix}background_{name}{texture_path_ext}";\n'
    else:
        result = f'\nconst std::string path_background_{name} = "";\n'
        for s in size_list:
            info_background.append(str(0)) # pos x
            info_background.append(str(0)) # pos y
            info_background.append(str(s[0])) # size x
            info_background.append(str(s[1])) # size y
    if(shadow): info_background.append(str(1))
    else: info_background.append(str(0))
    result += f"const uint16_t background_info_{name}[] = {{ " + ', '.join(info_background) + " }; \n\n"
 
    return result



def visual_console_data(name, path_console):
    img = Image.open(path_console)
    original_width, original_height = img.size
    
    # Calculate aspect ratios
    target_ratio = console_size[0] / console_size[1]
    image_ratio = original_width / original_height
    
    # Determine cuts for center crop to match target aspect ratio
    if image_ratio > target_ratio:
        # Image is wider than target - need to crop left and right
        new_width = int(original_height * target_ratio)
        x_cut_total = original_width - new_width
        x_cut_left = x_cut_total / 2
        x_cut_right = x_cut_total / 2
        y_cut = [0, 0]
    else:
        # Image is taller than target - need to crop top and bottom
        new_height = int(original_width / target_ratio)
        y_cut_total = original_height - new_height
        y_cut_top = y_cut_total / 2
        y_cut_bottom = y_cut_total / 2
        x_cut_left = 0
        x_cut_right = 0
        y_cut = [y_cut_top / original_height, y_cut_bottom / original_height]
    
    # Convert cuts to fractions
    x_cut = [x_cut_left / original_width, x_cut_right / original_width]
    
    img, tmp_x, tmp_y = im.transform_img(img, console_size[0], console_size[1], respect_ratio = True
                                         , rotate_90 = False, miror = True
                                         , new_ratio = 0, cut = [x_cut, y_cut], add_SHARPEN = False)
    img = np.array(img)
    img_f = np.zeros((console_atlas_size[1], console_atlas_size[0], 4), dtype=np.uint8)
    img_f[0:img.shape[0], 0:img.shape[1], :] = img
    Image.fromarray(img_f, mode="RGBA").save(destination_graphique_file + 'console_' + name + '.png')
    with open(destination_graphique_file + 'console_' + name + '.t3s', "w", encoding="utf-8") as f:
        if tex3ds_enabled:
            f.write("-f RGB8 -z none\n" + 'console_' + name + '.png')
        else:
            f.write("")
    pos_y = console_atlas_size[1]-img.shape[0]
    result = f'\nconst std::string path_console_{name} = "{texture_path_prefix}console_{name}{texture_path_ext}";\n'
    result += f"const uint16_t console_info_{name}[] = {{ {console_atlas_size[0]}, {console_atlas_size[1]}, 0, {pos_y}, {img.shape[1]}, {img.shape[0]} }}; \n\n"

    return result


def melody_text(name:str, melody_path:str):
    c_file = ""
    if(melody_path != ''):
        c_file += f"\nconst uint8_t melody_GW_{name}[] = "
        c_file += "{\n	"
        data = [0x00]
        with open(melody_path, "rb") as f: data = f.read() 
        hex_data = ", ".join([f"0x{byte:02X}" for byte in data])
        c_file += hex_data + '\n}; '
        c_file += f"const size_t size_melody_GW_{name} = sizeof(melody_GW_{name})/sizeof(melody_GW_{name}[0]);\n"
    else:
        c_file += f"\nconst uint8_t melody_GW_{name}[1] = "
        c_file += "{0}; \n"
        c_file += f"	const size_t size_melody_GW_{name} = 0;\n"
    return c_file


def rom_text(name:str, rom_path: str):
    c_file = ""
    data = [0x00]
    with open(rom_path, "rb") as f: data = f.read() 
    hex_data = ", ".join([f"0x{byte:02X}" for byte in data])
    c_file += f"\nconst uint8_t rom_GW_{name}[] = {{\n	" + hex_data + '\n}; '
    c_file += f"const size_t size_rom_GW_{name} = sizeof(rom_GW_{name})/sizeof(rom_GW_{name}[0]);\n"
    return c_file




def generate_game_file(destination_game_file, name, display_name, ref, date
                , rom_path, visual_path, size_visual, path_console
                , melody_path = '', background_path = [], rotate = False, mask = False, color_segment = False, two_in_one_screen = False
                , transform = [], alpha_bright = 1.7, fond_bright = 1.35, shadow = True):
    
    c_file = f"""
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "{name}.h"

"""
    c_file += rom_text(name, rom_path)    
    c_file += melody_text(name, melody_path)
    
    cs.extract_group_segs(visual_path, "./tmp/img/"+name, INKSCAPE_PATH, export_dpi=export_dpi)
    text, new_size_screen = visual_data_file(name, size_visual, background_path, rotate, mask, color_segment, two_in_one_screen, transform)
    c_file += text
    
    size_background = []
    for s in new_size_screen: size_background.append([int(s[0]), int(s[1])])
    if(two_in_one_screen): size_background[0] = size_background[1]
        
    if(mask): background_path = [] # background used for create segment
    c_file += background_data_file(name, background_path, size_background, rotate, alpha_bright, fond_bright, shadow)
    
    c_file += visual_console_data(name, path_console)
    
    c_file += "\n\n"
    
    c_file += f'''
const GW_rom {name} (
    "{display_name}", "{ref}", "{date}"
    , rom_GW_{name}, size_rom_GW_{name}
    , melody_GW_{name}, size_melody_GW_{name}
    , path_segment_{name}
    , segment_GW_{name}, size_segment_GW_{name}
    , segment_info_{name}
    , path_background_{name}
    , background_info_{name}
    , path_console_{name}
    , console_info_{name}
);

'''   
    with open(destination_game_file+"\\"+name+".cpp", "w") as f: f.write(c_file)
    
    # generate h file
    c_file = f"""
#pragma once
#include "GW_ROM.h"

extern const GW_rom {name};

"""    
    with open(destination_game_file+"\\"+name+".h", "w") as f: f.write(c_file)
    return 



def generate_global_file(games_path, destination_file):
    c_file_final = """
#pragma once

"""
    for key in games_path:
        c_file_final += f'#include "{gw_rom_include_dir}/{key}.h"\nextern const GW_rom {key};\n'

    c_file_final += "\n\n\n\n"
    c_file_final += 'const GW_rom* GW_list[] = {&' + ', &'.join(games_path.keys()) + '};\n'
    c_file_final += 'const size_t nb_games = ' + str(len(games_path.keys()))+ ';\n\n'
    
    with open(destination_file, "w") as f: f.write(c_file_final)




def validate_game_files(games_path):
    """Check if all required files exist for each game before processing."""
    print("Validating game files...")
    print("=" * 60)
    
    missing_files = {}
    has_errors = False
    
    for key in games_path:
        game_missing = []
        
        # Check ROM file
        rom_path = games_path[key]["Rom"]
        if not os.path.exists(rom_path):
            game_missing.append(f"ROM: {rom_path}")
        
        # Check Melody ROM if specified
        if 'Melody_Rom' in games_path[key] and games_path[key]['Melody_Rom'] != '':
            melody_path = games_path[key]['Melody_Rom']
            if not os.path.exists(melody_path):
                game_missing.append(f"Melody: {melody_path}")
        
        # Check Visual files (SVG files)
        if 'Visual' in games_path[key]:
            for visual_path in games_path[key]['Visual']:
                if not os.path.exists(visual_path):
                    game_missing.append(f"Visual: {visual_path}")
        
        # Check Background files if specified
        if 'Background' in games_path[key] and len(games_path[key]['Background']) > 0:
            for bg_path in games_path[key]['Background']:
                if bg_path != '' and not os.path.exists(bg_path):
                    game_missing.append(f"Background: {bg_path}")
        
        # Check Console file
        console_path = games_path[key].get('console', default_console)
        if not os.path.exists(console_path):
            game_missing.append(f"Console: {console_path}")
        
        # Store missing files for this game
        if game_missing:
            missing_files[key] = game_missing
            has_errors = True
    
    # Report results
    if has_errors:
        print("\n❌ MISSING FILES DETECTED:\n")
        for game, files in missing_files.items():
            print(f"Game: {game}")
            for file in files:
                print(f"  - {file}")
            print()
        print("=" * 60)
        print(f"\n⚠️  Found {len(missing_files)} game(s) with missing files.")
        print("Please add the missing files before running the script.\n")
        return False
    else:
        print("✓ All required files found!")
        print("=" * 60)
        print()
        return True


def process_single_game(args):
    """Process a single game - designed for multiprocessing or sequential use."""
    key, game_data = args
    
    print(f"\n--------\n{key}\n")

    if reset_img_svg:
        try: 
            # Remove existing ./tmp/img/<game> directory if it exists
            shutil.rmtree("./tmp/img/" + key)
            print(f"Removed cache folder: tmp/img/{key}")

            # Clean up gfx for this game: remove all .t3s and .png files
            # whose filenames contain the current game key, using glob
            if os.path.exists(destination_graphique_file):
                pattern = os.path.join(destination_graphique_file, f"*{key}*")
                for file_path in glob.glob(pattern):
                    filename = os.path.basename(file_path)
                    try:
                        os.remove(file_path)
                        print(f"Removed: {filename}")
                    except Exception as e:
                        print(f"Error removing {filename}: {e}")
        except: 
            pass
                
    # Set default values if not exist
    alpha_bright = game_data.get('alpha_bright', default_alpha_bright)
    fond_bright = game_data.get('fond_bright', default_fond_bright)
    rotate = game_data.get('rotate', default_rotate)
    mask = game_data.get('mask', False)
    color_segment = game_data.get('color_segment', False)
    two_in_one_screen = game_data.get('2_in_one_screen', False)
    melody_path = game_data.get('Melody_Rom', '')
    background_path = game_data.get('Background', [])
    size_visual = game_data.get('size_visual', [resolution_up, resolution_down])
    if size_scale != 1:
        size_visual = [[int(s[0] * size_scale), int(s[1] * size_scale)] for s in size_visual]
    path_console = game_data.get('console', default_console)
    display_name = game_data.get('display_name', key)
    shadow = game_data.get('shadow', True)
    date = game_data.get('date', '198X-XX-XX')

    generate_game_file(
        destination_game_file, key, display_name,
        game_data["ref"].replace('-', '_').upper(), date,
        game_data["Rom"], game_data["Visual"], size_visual,
        path_console, melody_path, background_path,
        rotate, mask, color_segment, two_in_one_screen,
        game_data["transform_visual"],
        alpha_bright, fond_bright, shadow
    )
    
    return key


if __name__ == "__main__":
    import argparse
    clean_games_path()

    parser = argparse.ArgumentParser(description="Build Game & Watch assets")
    parser.add_argument(
        "--target",
        default="3ds",
        help="Target profile (e.g. '3ds' or 'rgds').",
    )
    parser.add_argument(
        "--out-gfx",
        default=None,
        help="Output folder for generated PNG/T3S (default depends on target).",
    )
    parser.add_argument(
        "--out-gw-all",
        default=None,
        help="Output GW_ALL header path (default depends on target).",
    )
    parser.add_argument(
        "--out-gw-rom-dir",
        default=None,
        help="Output folder for per-game .cpp/.h (default depends on target).",
    )
    parser.add_argument(
        "--export-dpi",
        type=int,
        default=None,
        help="Override Inkscape export DPI (default depends on target).",
    )
    parser.add_argument(
        "--scale",
        type=int,
        default=None,
        help="Override size scale multiplier (default depends on target).",
    )
    parser.add_argument(
        "--game",
        "-g",
        metavar="NAME",
        help="Only rebuild a single game (use key from games_path, e.g. 'cgrab' or 'Fire')",
    )
    parser.add_argument(
        "--parallel",
        dest="use_parallel",
        action="store_true",
        help="Enable multiprocessing when building multiple games",
    )
    parser.add_argument(
        "-c",
        "--clean",
        dest="reset_img_svg",
        action="store_true",
        help="Delete and regenerate ./tmp/img/<game> before processing",
    )

    args = parser.parse_args()

    apply_profile(args.target, args.out_gfx, args.out_gw_all, args.out_gw_rom_dir, args.export_dpi, args.scale)

    # Load games_path based on target.
    games_path = _load_games_path_for_target(args.target)

    os.makedirs(destination_game_file, exist_ok=True)
    os.makedirs(destination_graphique_file, exist_ok=True)

    # Toggle to control processing mode
    USE_PARALLEL_PROCESSING = args.use_parallel  # default False unless --parallel is passed

    # Control whether we reset ./tmp/img/<game> directories before processing
    # If -c/--clean is passed, we enable cleaning; otherwise we leave the
    # module-level default value unchanged.
    if args.reset_img_svg:
        reset_img_svg = True

    # Validate all game files before processing
    if not validate_game_files(games_path):
        print("Exiting due to missing files.")
        exit(1)
    
    os.makedirs(r'.\tmp', exist_ok=True)
    os.makedirs(r'.\tmp\img', exist_ok=True)

    # Prepare game data with default values (optionally for a single game)
    game_items = []

    # If a specific game was requested, only process that one
    if args.game:
        args.game = str(args.game).lower()
        if args.game not in games_path:
            print(f"Unknown game '{args.game}'. Available keys:")
            for k in sorted(games_path.keys()):
                print(f"  - {k}")
            exit(1)
        keys_to_process = [args.game]
    else:
        keys_to_process = list(games_path.keys())

    for key in keys_to_process:
        game_data = games_path[key].copy()
        
        # Set defaults
        if 'alpha_bright' not in game_data: game_data['alpha_bright'] = default_alpha_bright
        if 'fond_bright' not in game_data: game_data['fond_bright'] = default_fond_bright
        if 'rotate' not in game_data: game_data['rotate'] = default_rotate
        if 'mask' not in game_data: game_data['mask'] = False
        if 'color_segment' not in game_data: game_data['color_segment'] = False
        if '2_in_one_screen' not in game_data: game_data['2_in_one_screen'] = False
        if 'Melody_Rom' not in game_data: game_data['Melody_Rom'] = ''
        if 'Background' not in game_data: game_data['Background'] = []
        if 'size_visual' not in game_data: game_data['size_visual'] = [resolution_up, resolution_down]
        if 'console' not in game_data: game_data['console'] = default_console
        if 'display_name' not in game_data: game_data['display_name'] = key
        if 'shadow' not in game_data: game_data['shadow'] = True 
        if 'date' not in game_data: game_data['date'] = '198X-XX-XX'
        
        game_items.append((key, game_data))
    
    if USE_PARALLEL_PROCESSING:
        # Process games in parallel using multiprocessing
        # Adjust the number of processes based on your CPU cores
        # Use fewer processes if you run into memory issues
        num_processes = min(4, os.cpu_count() or 1)  # Use up to 4 processes

        print(f"\n{'='*60}")
        print(f"Processing {len(game_items)} games using {num_processes} parallel processes...")
        print(f"{'='*60}\n")

        try:
            with Pool(processes=num_processes) as pool:
                results = pool.map(process_single_game, game_items)

            print(f"\n{'='*60}")
            print(f"Successfully processed {len(results)} games!")
            print(f"{'='*60}\n")

        except Exception as e:
            print(f"\n❌ Error during parallel processing: {e}")
            print("Falling back to sequential processing...\n")

            # Fallback to sequential processing
            for item in game_items:
                try:
                    process_single_game(item)
                except Exception as game_error:
                    print(f"Error processing {item[0]}: {game_error}")
                    continue
    else:
        # Sequential processing (default)
        print(f"\n{'='*60}")
        print(f"Processing {len(game_items)} games sequentially...")
        print(f"{'='*60}\n")

        results = []
        for item in game_items:
            try:
                result = process_single_game(item)
                results.append(result)
            except Exception as game_error:
                print(f"Error processing {item[0]}: {game_error}")
                continue

    generate_global_file(games_path, destination_file)
        
    print("\n\n\n\n------------------------------------------------------ Finish !!!!!!!!!!!!")
