import os
import glob
import shutil
import importlib.util
import struct
import subprocess
from pathlib import Path
from PIL import Image, ImageEnhance
import numpy as np
from rectpack import newPacker
from multiprocessing import Pool
import functools
from typing import Iterable

from source import convert_svg as cs
from source import img_manipulation as im
from source import game_cache
from source.target_profiles import get_target
from source.manufacturer_ids import (
    MANUFACTURER_NINTENDO,
    MANUFACTURER_TRONICA,
    normalize_manufacturer_id,
)

ROMPACK_FORMAT_VERSION = 3
ROMPACK_CONTENT_VERSION = 3

# Configurable external apps.
# Configure them by copying CONVERT_ROM/external_apps_template.py -> CONVERT_ROM/external_apps.py.
try:
    from external_apps import INKSCAPE_PATH, TEX3DS_PATH  # type: ignore
except Exception:
    # Defaults
    INKSCAPE_PATH = r"C:\Program Files\Inkscape\bin\inkscape.exe"
    TEX3DS_PATH = r"C:\devkitPro\tools\bin\tex3ds.exe"

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

# Cache behavior:
# - Cache files are always written after a successful rebuild (best-effort).
# - Reading cache to skip rebuilding is only enabled with --use-cache.
USE_CACHE_READ = False


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


def clean_games_path(games_path):
    tmp_gamepath = {}
    for key in games_path:
        new_key = str(key).lower()
        if(new_key not in tmp_gamepath):
            tmp_gamepath[new_key] = games_path[key]
    return tmp_gamepath



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



def extract_color_index(filename: str) -> int:
    parts = filename.split("_")
    if len(parts) > 1:
        try:
            return int(parts[1].split(".")[0])
        except Exception as e:
            print(f"[ERROR] Could not parse color_index from filename: {filename}")
            print(f"  parts: {parts}")
            print(f"  Exception: {e}")
            return 0
    else:
        print(f"[ERROR] Filename does not contain expected '_': {filename}")
        return 0


def segment_text(all_img, name, color_segment:bool = False):
    result = f"\nconst Segment segment_GW_{name}[] = {{\n\t"
    for data in all_img:
        filename, img_r, screen, pos_x, pos_y, size_x, size_y, pos_x_tex, pos_y_tex = data
        seg_x = int(filename.split(".")[0])
        seg_y = int(filename.split(".")[1])
        seg_z = int(filename.split(".")[2].split("_")[0])
        color_index = extract_color_index(filename) if color_segment else 0
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
    seg_info_ints: list[int] = [int(atlas_size[0]), int(atlas_size[1]), 1, int(int_mask)]

    for src in screen_size:
        seg_info_ints.extend([int(src[0]), int(src[1])])

    result += f" const uint16_t segment_info_{name}[] = {{ " + ", ".join(str(i) for i in seg_info_ints) + "}; \n"

    # Pack-friendly segment list (matches SegmentDiskV1 in source/std/gw_pack.cpp)
    segment_records: list[dict] = []
    for data in all_img:
        filename, _img_r, screen, pos_x, pos_y, size_x, size_y, pos_x_tex, pos_y_tex = data
        seg_x = int(filename.split(".")[0])
        seg_y = int(filename.split(".")[1])
        seg_z = int(filename.split(".")[2].split("_")[0])
        color_index = extract_color_index(filename) if color_segment else 0
        segment_records.append({
            "id0": seg_x,
            "id1": seg_y,
            "id2": seg_z,
            "pos_scr_x": int(pos_x),
            "pos_scr_y": int(pos_y),
            "pos_tex_x": int(pos_x_tex),
            "pos_tex_y": int(pos_y_tex),
            "size_tex_x": int(size_x),
            "size_tex_y": int(size_y),
            "color_index": int(color_index),
            "screen": int(screen),
        })

    return result, screen_size, seg_info_ints, segment_records



def background_data_file(name, path_list = [], size_list = [], rotate = False, alpha_bright = 1.7, fond_bright = 1.35
                            , shadow = True, background_in_front = False, camera = False):
    i = 0
    atlas_size = [1, 1]
    for size in size_list:
        atlas_size[0] = max(1+atlas_size[0]+1, size[0])
        atlas_size[1] += size[1]+2
   
    for i_a in range(2):
        atlas_size[i_a] = _round_up_to_one_of(atlas_size[i_a], background_atlas_sizes or [atlas_size[i_a]])
        
    result_img = np.zeros((atlas_size[1], atlas_size[0], 4))
    curr_ind_r_img = 1
    info_background_ints: list[int] = [int(atlas_size[0]), int(atlas_size[1])]
    
    if(len(path_list) > 0 and path_list[0] != ''):
        for path in path_list:
            if(rotate): y_size, x_size  = size_list[i]
            else: x_size, y_size = size_list[i]
                
            img = Image.open(path)

            img, x_size, y_size = im.transform_img(img, x_size, y_size, False, rotate, True)
            data = im.make_alpha(img, fond_bright, alpha_bright)

            result_img[curr_ind_r_img:(data.shape[0]+curr_ind_r_img), 1:data.shape[1]+1, :] = data
            
            info_background_ints.append(1) # pos x
            info_background_ints.append(int(atlas_size[1]-curr_ind_r_img-data.shape[0])) # pos y
            info_background_ints.append(int(data.shape[1])) # size x
            info_background_ints.append(int(data.shape[0])) # size y
            curr_ind_r_img = data.shape[0]+2
            i += 1

        if(camera): # create alpha
            result_img[:, :, 3] = (result_img[:, :, 3]*0.5).astype(np.uint8)
        
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
            info_background_ints.append(0) # pos x
            info_background_ints.append(0) # pos y
            info_background_ints.append(int(s[0])) # size x
            info_background_ints.append(int(s[1])) # size y
    info_background_ints.append(1 if shadow else 0)
    info_background_ints.append(1 if background_in_front else 0)
    info_background_ints.append(1 if camera else 0)
    result += f"const uint16_t background_info_{name}[] = {{ " + ', '.join(str(i) for i in info_background_ints) + " }; \n\n"
 
    return result, info_background_ints



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
    console_info_ints: list[int] = [
        int(console_atlas_size[0]),
        int(console_atlas_size[1]),
        0,
        int(pos_y),
        int(img.shape[1]),
        int(img.shape[0]),
    ]
    result += f"const uint16_t console_info_{name}[] = {{ " + ", ".join(str(i) for i in console_info_ints) + "}; \n\n"

    return result, console_info_ints


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


def _manufacturer_to_id(value) -> int:
    """Normalize manufacturer values into a small numeric id."""

    return normalize_manufacturer_id(value, default=MANUFACTURER_NINTENDO)




def generate_game_file(destination_game_file, name, display_name, ref, date
                , rom_path, visual_path, size_visual, path_console
                , melody_path = '', background_path = [], rotate = False, mask = False, color_segment = False, two_in_one_screen = False
                , transform = [], alpha_bright = 1.7, fond_bright = 1.35, shadow = True
                , background_in_front = False, camera = False, manufacturer = MANUFACTURER_NINTENDO):
    
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
    text, new_size_screen, seg_info_ints, segment_records = visual_data_file(
        name, size_visual, background_path, rotate, mask, color_segment, two_in_one_screen, transform
    )
    c_file += text
    
    size_background = []
    for s in new_size_screen: size_background.append([int(s[0]), int(s[1])])
    if(two_in_one_screen): size_background[0] = size_background[1]
        
    if(mask): background_path = [] # background used for create segment
    bg_text, background_info_ints = background_data_file(
        name,
        background_path,
        size_background,
        rotate,
        alpha_bright,
        fond_bright,
        shadow,
        background_in_front,
        camera,
    )
    c_file += bg_text
    
    cs_text, console_info_ints = visual_console_data(name, path_console)
    c_file += cs_text
    
    c_file += "\n\n"
    
    manufacturer_id = _manufacturer_to_id(manufacturer)
    manufacturer_cpp = "GW_rom::MANUFACTURER_TRONICA" if manufacturer_id == MANUFACTURER_TRONICA else "GW_rom::MANUFACTURER_NINTENDO"

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
    , {manufacturer_cpp}
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

    # Pack metadata (used to generate yokoi_pack_<target>.ykp)
    return {
        "key": name,
        "display_name": display_name,
        "ref": ref,
        "date": date,
        "manufacturer": manufacturer_id,
        "rom_path": rom_path,
        "melody_path": melody_path,
        "path_segment": f"{texture_path_prefix}segment_{name}{texture_path_ext}",
        "path_background": f"{texture_path_prefix}background_{name}{texture_path_ext}" if background_path else "",
        "path_console": f"{texture_path_prefix}console_{name}{texture_path_ext}",
        "segments": segment_records,
        "segment_info": seg_info_ints,
        "background_info": background_info_ints,
        "console_info": console_info_ints,
    }



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
        # Best-effort cleanup. Each step is isolated so a missing folder doesn't
        # prevent cache invalidation or gfx cleanup.

        # Remove existing ./tmp/img/<game> directory if it exists
        try:
            shutil.rmtree("./tmp/img/" + key)
            print(f"Removed cache folder: tmp/img/{key}")
        except FileNotFoundError:
            pass
        except Exception as e:
            print(f"Warning: unable to remove tmp/img/{key}: {e}")

        # Also invalidate the per-game build cache (even if tmp/img didn't exist).
        deleted_cache = game_cache.invalidate_game(key)
        if deleted_cache is not None:
            print(f"Removed cache file: {deleted_cache}")

        # Clean up gfx for this game: remove all .t3s/.png/.t3x files
        # whose filenames contain the current game key.
        if os.path.exists(destination_graphique_file):
            pattern = os.path.join(destination_graphique_file, f"*{key}*")
            for file_path in glob.glob(pattern):
                filename = os.path.basename(file_path)
                try:
                    os.remove(file_path)
                    print(f"Removed: {filename}")
                except Exception as e:
                    print(f"Error removing {filename}: {e}")
                
    # Set default values if not exist
    alpha_bright = game_data.get('alpha_bright', default_alpha_bright)
    fond_bright = game_data.get('fond_bright', default_fond_bright)
    rotate = game_data.get('rotate', default_rotate)
    mask = game_data.get('mask', False)
    color_segment = game_data.get('color_segment', False)
    two_in_one_screen = game_data.get('2_in_one_screen', False)
    melody_path = game_data.get('Melody_Rom', '')
    background_path = game_data.get('Background', [])
    background_in_front = game_data.get('background_in_front', False)
    camera = game_data.get('camera', False)
    size_visual = game_data.get('size_visual', [resolution_up, resolution_down])
    if size_scale != 1:
        size_visual = [[int(s[0] * size_scale), int(s[1] * size_scale)] for s in size_visual]
    path_console = game_data.get('console', default_console)
    display_name = game_data.get('display_name', key)
    shadow = game_data.get('shadow', True)
    date = game_data.get('date', '198X-XX-XX')

    ref_norm = game_data["ref"].replace('-', '_').upper()
    manufacturer = _manufacturer_to_id(game_data.get("manufacturer", 0))

    if USE_CACHE_READ:
        cached = game_cache.try_load_pack_meta(key, game_data, clean_mode=reset_img_svg)
        if cached is not None:
            print(f"(cache) Up-to-date: {key}")
            return cached

    pack_meta = generate_game_file(
        destination_game_file, key, display_name,
        ref_norm, date,
        game_data["Rom"], game_data["Visual"], size_visual,
        path_console, melody_path, background_path,
        rotate, mask, color_segment, two_in_one_screen,
        game_data["transform_visual"],
        alpha_bright, fond_bright, shadow,
        background_in_front, camera,
        manufacturer
    )

    wrote_cache = game_cache.write_game_cache(key, game_data, pack_meta)
    if wrote_cache is not None:
        print(f"(cache) Wrote: {wrote_cache}")
    
    return pack_meta


def _build_tex3ds_outputs_for_pack(t3s_dir: Path, t3x_out_dir: Path) -> None:
    """Build .t3x files from .t3s inputs using tex3ds.

    convert_3ds.py generates *.t3s + *.png, but the 3DS pack bundles *.t3x.
    This helper makes pack generation self-contained by invoking tex3ds directly.
    """

    t3s_dir = t3s_dir.resolve()
    t3x_out_dir = t3x_out_dir.resolve()
    t3x_out_dir.mkdir(parents=True, exist_ok=True)

    t3s_files = sorted(t3s_dir.glob("*.t3s"))
    if not t3s_files:
        raise RuntimeError(
            f"No .t3s files found in '{t3s_dir}'. "
            "Expected generated texture descriptors (segment_*.t3s/background_*.t3s/console_*.t3s)."
        )

    for t3s in t3s_files:
        out_t3x = t3x_out_dir / f"{t3s.stem}.t3x"

        try:
            needs_build = (not out_t3x.exists()) or (out_t3x.stat().st_mtime < t3s.stat().st_mtime)
        except OSError:
            needs_build = True

        if not needs_build:
            continue

        try:
            # Run with cwd=t3s_dir so relative PNG references in the .t3s resolve reliably.
            subprocess.run(
                [TEX3DS_PATH, "-i", t3s.name, "-o", str(out_t3x)],
                cwd=str(t3s_dir),
                check=True,
                capture_output=True,
                text=True,
            )
        except FileNotFoundError as e:
            raise RuntimeError(
                f"external app tex3ds not found (configured as '{TEX3DS_PATH}'); "
                "check CONVERT_ROM/external_apps.py (copy from external_apps_template.py), or put tex3ds on PATH."
            ) from e
        except subprocess.CalledProcessError as e:
            stderr = (e.stderr or "").strip()
            stdout = (e.stdout or "").strip()
            details = stderr or stdout or str(e)
            raise RuntimeError(f"tex3ds failed for '{t3s.name}': {details}") from e

def write_rom_pack_v1(pack_games: list[dict], gfx_dir: str, out_path: str, platform: int = 0, texture_file_ext: str = ".png"):
    """Write a single external ROM pack.

    Note: The pack "format" version may change independently from the pack "content" version.
    The content version is used by the apps to detect outdated packs and prompt import only
    when needed.
    """

    # Collect unique texture files referenced by games.
    file_name_to_path: dict[str, str] = {}
    for g in pack_games:
        key = g["key"]
        for base in (f"segment_{key}{texture_file_ext}", f"background_{key}{texture_file_ext}", f"console_{key}{texture_file_ext}"):
            p = os.path.join(gfx_dir, base)
            if os.path.exists(p):
                file_name_to_path[base] = p

    # Include UI textures that the 3DS frontend needs to render text/credits.
    # These are safe to bundle and allow ROMPACK_ONLY builds to work without embedding romfs.
    for base in (f"texte_3ds{texture_file_ext}", f"logo_pioupiou{texture_file_ext}"):
        p = os.path.join(gfx_dir, base)
        if os.path.exists(p):
            file_name_to_path[base] = p

    file_items = sorted(file_name_to_path.items(), key=lambda kv: kv[0])

    if not file_items:
        raise RuntimeError(
            f"No texture files found for pack in '{gfx_dir}' (expected files like segment_*{texture_file_ext}). "
            "Build/prepare textures first, then re-run convert_3ds.py."
        )

    header_size = 36  # PackHeaderV2
    game_entry_size = 100  # GameEntryV2 (25 * uint32)
    file_entry_size = 16  # FileEntryV1

    games_offset = header_size
    files_offset = games_offset + (len(pack_games) * game_entry_size)
    data_offset = files_offset + (len(file_items) * file_entry_size)

    data = bytearray()

    def append_chunk(b: bytes) -> int:
        off = data_offset + len(data)
        data.extend(b)
        return off

    def append_string(s: str) -> tuple[int, int]:
        b = (s or "").encode("utf-8")
        return append_chunk(b), len(b)

    def append_u16_list(vals: list[int]) -> tuple[int, int]:
        if not vals:
            return 0, 0
        packed = struct.pack("<" + "H" * len(vals), *[int(v) & 0xFFFF for v in vals])
        return append_chunk(packed), len(vals)

    def append_segments(segs: list[dict]) -> tuple[int, int]:
        if not segs:
            return 0, 0
        out = bytearray()
        for s in segs:
            out.extend(struct.pack(
                "<BBBiiHHHHBB",
                int(s["id0"]) & 0xFF,
                int(s["id1"]) & 0xFF,
                int(s["id2"]) & 0xFF,
                int(s["pos_scr_x"]),
                int(s["pos_scr_y"]),
                int(s["pos_tex_x"]) & 0xFFFF,
                int(s["pos_tex_y"]) & 0xFFFF,
                int(s["size_tex_x"]) & 0xFFFF,
                int(s["size_tex_y"]) & 0xFFFF,
                int(s["color_index"]) & 0xFF,
                int(s["screen"]) & 0xFF,
            ))
        return append_chunk(bytes(out)), len(segs)

    game_entries: list[bytes] = []
    for g in pack_games:
        name_off, name_len = append_string(g["display_name"])
        ref_off, ref_len = append_string(g["ref"])
        date_off, date_len = append_string(g["date"])

        with open(g["rom_path"], "rb") as f:
            rom_bytes = f.read()
        rom_off = append_chunk(rom_bytes)
        rom_size = len(rom_bytes)

        melody_off = 0
        melody_size = 0
        melody_path = g.get("melody_path") or ""
        if melody_path:
            with open(melody_path, "rb") as f:
                melody_bytes = f.read()
            melody_off = append_chunk(melody_bytes)
            melody_size = len(melody_bytes)

        path_segment_off, path_segment_len = append_string(g["path_segment"])
        segments_off, segments_count = append_segments(g["segments"])
        segment_info_off, segment_info_count = append_u16_list(g["segment_info"])

        path_background_off, path_background_len = append_string(g["path_background"])
        background_info_off, background_info_count = append_u16_list(g["background_info"])

        path_console_off, path_console_len = append_string(g["path_console"])
        console_info_off, console_info_count = append_u16_list(g["console_info"])

        manufacturer_id = _manufacturer_to_id(g.get("manufacturer", MANUFACTURER_NINTENDO))

        game_entries.append(struct.pack(
            "<" + "I" * 25,
            name_off, name_len,
            ref_off, ref_len,
            date_off, date_len,
            rom_off, rom_size,
            melody_off, melody_size,
            path_segment_off, path_segment_len,
            segments_off, segments_count,
            segment_info_off, segment_info_count,
            path_background_off, path_background_len,
            background_info_off, background_info_count,
            path_console_off, path_console_len,
            console_info_off, console_info_count,
            manufacturer_id,
        ))

    file_entries: list[bytes] = []
    for name, path in file_items:
        n_off, n_len = append_string(name)
        with open(path, "rb") as f:
            blob = f.read()
        d_off = append_chunk(blob)
        d_size = len(blob)
        file_entries.append(struct.pack("<IIII", n_off, n_len, d_off, d_size))

    header = struct.pack(
        "<IIIIIIIII",
        0x31504B59,                   # 'YKP1'
        int(ROMPACK_FORMAT_VERSION),  # format version
        int(platform),
        int(ROMPACK_CONTENT_VERSION),
        len(pack_games),
        len(file_entries),
        games_offset,
        files_offset,
        data_offset,
    )

    with open(out_path, "wb") as f:
        f.write(header)
        for e in game_entries:
            f.write(e)
        for e in file_entries:
            f.write(e)
        f.write(data)


if __name__ == "__main__":
    import argparse

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
        "--sort",
        default="none",
        choices=["none", "key", "display_name", "date", "ref"],
        help=(
            "Optional deterministic ordering for games in the generated pack. "
            "This affects the menu order (pack entry order). Default 'none' preserves games_path order."
        ),
    )
    parser.add_argument(
        "--sort-reverse",
        action="store_true",
        help="Reverse the selected --sort ordering.",
    )
    parser.add_argument(
        "-c",
        "--clean",
        dest="reset_img_svg",
        action="store_true",
        help="Delete and regenerate ./tmp/img/<game> before processing, and invalidate the per-game cache entry",
    )

    parser.add_argument(
        "--use-cache",
        action="store_true",
        help="Use per-game cache to skip rebuilding unchanged games (cache files are still written after rebuilds)",
    )

    args = parser.parse_args()

    print(f"\n=== convert_3ds.py target: {args.target} ===\n")

    apply_profile(args.target, args.out_gfx, args.out_gw_all, args.out_gw_rom_dir, args.export_dpi, args.scale)

    # Always enable cache *writing* so rebuilt games produce fresh cache files.
    # Cache *reading* (skip rebuilds) is controlled separately by --use-cache.
    cache_enabled = True
    USE_CACHE_READ = bool(args.use_cache)
    game_cache.configure(
        enabled=cache_enabled,
        target_name=str(args.target),
        cache_dir=Path(r".\\tmp\\cache"),
        export_dpi=export_dpi,
        size_scale=size_scale,
        resolution_up=resolution_up,
        resolution_down=resolution_down,
        console_size=console_size,
        console_atlas_size=console_atlas_size,
        tex3ds_enabled=tex3ds_enabled,
        texture_path_prefix=texture_path_prefix,
        texture_path_ext=texture_path_ext,
        destination_game_file=destination_game_file,
        destination_graphique_file=destination_graphique_file,
        default_console=default_console,
        default_alpha_bright=default_alpha_bright,
        default_fond_bright=default_fond_bright,
        default_rotate=default_rotate,
    )

    # Load games_path based on target.
    games_path = _load_games_path_for_target(args.target)
    games_path = clean_games_path(games_path)

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
    game_cache.ensure_cache_dir()

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

        if args.sort != "none":
            def _sort_key(k: str):
                g = games_path.get(k, {})
                if args.sort == "key":
                    return k
                if args.sort == "display_name":
                    return str(g.get("display_name", k))
                if args.sort == "date":
                    # Expect YYYY-MM-DD; string sort works for this format.
                    return str(g.get("date", ""))
                if args.sort == "ref":
                    return str(g.get("ref", ""))
                return k

            keys_to_process = sorted(keys_to_process, key=_sort_key, reverse=bool(args.sort_reverse))

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
            results = []
            for item in game_items:
                try:
                    result = process_single_game(item)
                    results.append(result)
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

    # Always write a pack for the selected target.
    script_dir = Path(__file__).resolve().parent
    canonical_pack_path = script_dir / f"yokoi_pack_{args.target}.ykp"
    versioned_pack_path = script_dir / f"yokoi_pack_{args.target}_v{ROMPACK_FORMAT_VERSION}.{ROMPACK_CONTENT_VERSION}.ykp"

    # Target-specific defaults.
    platform_id = 0
    texture_file_ext = ".png"
    if args.target == "3ds":
        platform_id = 1
        texture_file_ext = ".t3x"
    elif args.target == "rgds":
        platform_id = 2
        texture_file_ext = ".png"

    # For 3DS packs we bundle .t3x, so ensure tex3ds has produced them.
    if args.target == "3ds":
        _build_tex3ds_outputs_for_pack(
            t3s_dir=Path(destination_graphique_file),
            t3x_out_dir=(script_dir.parent / "romfs" / "gfx"),
        )

    # Texture source directory for pack:
    # - 3DS: always use ../romfs/gfx (contains built .t3x).
    # - RGDS: use generated output dir (gfx2x by default).
    if args.target == "3ds":
        candidate = (script_dir.parent / "romfs" / "gfx")
        if not candidate.exists():
            raise RuntimeError(
                f"3DS pack requires built .t3x textures in '{candidate}'. "
                "Build the 3DS assets (tex3ds) first so romfs/gfx is populated, then re-run convert_3ds.py."
            )
        pack_gfx_dir = str(candidate)
    else:
        pack_gfx_dir = destination_graphique_file

    print(f"\nWriting ROM pack (versioned): {versioned_pack_path}")
    print(f"Writing ROM pack (canonical):  {canonical_pack_path}")
    print(f"Pack textures from: {pack_gfx_dir}")
    pack_games = [r for r in results if isinstance(r, dict)]
    # Write the versioned file for humans, and also keep the canonical filename that the apps load.
    write_rom_pack_v1(pack_games, pack_gfx_dir, str(versioned_pack_path), platform=platform_id, texture_file_ext=texture_file_ext)
    try:
        import shutil
        shutil.copyfile(versioned_pack_path, canonical_pack_path)
    except Exception as e:
        raise RuntimeError(f"Failed to write canonical pack '{canonical_pack_path}': {e}")
        
    print("\n\n\n\n------------------------------------------------------ Finish !!!!!!!!!!!!")
