from PIL import Image, ImageEnhance, ImageFilter
import numpy as np

seuil_tr = 170 # transparency
seuil_semi_tr = 100 # semi-transparency

# Alpha cutoff used throughout transforms.
_ALPHA_CUTOFF = 30
_ALPHA_LUT_30 = [0 if i < _ALPHA_CUTOFF else i for i in range(256)]



################ Convert PNG to Emulator ############################################
def transform_img(image_b, x_size, y_size, respect_ratio, rotate_90, miror
                  , new_ratio = 0, cut = [[0, 0],[0, 0]], add_SHARPEN=False):
    image_base = image_b.convert("RGBA")

    # Delete low alpha (fast LUT, avoids NumPy roundtrip).
    a = image_base.getchannel("A").point(_ALPHA_LUT_30)
    image_base.putalpha(a)

    w, h = image_base.size

    # Calcul cuts en pixels
    x_cut_start = int(cut[0][0] * w)
    x_cut_end   = int(cut[0][1] * w)
    if new_ratio != 0:
        y_cut_start = int(cut[1][0] * w / new_ratio)
        y_cut_end   = int(cut[1][1] * w / new_ratio)
    else:
        y_cut_start = int(cut[1][0] * h)
        y_cut_end   = int(cut[1][1] * h)

    # Crop calculé correctement
    x_start = max(0, x_cut_start)
    y_start = max(0, y_cut_start)
    x_end = max(0, x_cut_end)
    y_end = max(0, y_cut_end)

    # Nouvelle largeur / hauteur
    new_w = w - x_start - x_end
    new_h = h - y_start - y_end

    # Canvas largeur / hauteur pour cuts négatifs à gauche/haut seulement
    canvas_w = new_w + max(-x_cut_start,0) + max(-x_cut_end,0)
    canvas_h = new_h + max(-y_cut_start,0) + max(-y_cut_end,0)
    new_image = Image.new("RGBA", (canvas_w, canvas_h), (0,0,0,0))

    # Position pour coller l'image
    paste_x = max(-x_cut_start, 0)
    paste_y = max(-y_cut_start, 0)

    # Crop limité aux pixels valides
    crop_box = (x_start, y_start, x_start + new_w, y_start + new_h)
    cropped = image_base.crop(crop_box)

    new_image.paste(cropped, (paste_x, paste_y))
    image_base = new_image

    # Miroir / rotation
    if miror:
        image_base = image_base.transpose(Image.FLIP_LEFT_RIGHT)
    if rotate_90:
        image_base = image_base.rotate(-90, expand=True)
        x_size, y_size = y_size, x_size
        if new_ratio != 0: new_ratio = 1/new_ratio

    # Resize en respectant ratio
    if new_ratio != 0:
        y_size = min(y_size, int(x_size / new_ratio))
        x_size = min(x_size, int(y_size * new_ratio))
    elif respect_ratio:
        ratio = image_base.size[0] / image_base.size[1]
        y_size = min(y_size, int(x_size / ratio))
        x_size = min(x_size, int(y_size * ratio))

    img = image_base.resize((x_size, y_size), resample=Image.LANCZOS)

    if add_SHARPEN: img = img.filter(ImageFilter.UnsharpMask(radius=1, percent=50, threshold=3))

    # Reapply alpha cutoff after resize (fast LUT).
    a = img.getchannel("A").point(_ALPHA_LUT_30)
    img.putalpha(a)

    return img, x_size, y_size

    
    
def find_x_y_size(img_base):
    # Use Pillow's bbox on alpha channel (C-accelerated), avoids NumPy scan.
    alpha = img_base.getchannel("A")
    bbox = alpha.getbbox()
    if bbox is None:
        return 0, 0, 0, 0
    left, upper, right, lower = bbox  # right/lower are exclusive
    return left, right - 1, upper, lower - 1


def crop_img(img_base):
    x_min, x_max, y_min, y_max = find_x_y_size(img_base)
    # Keep legacy behavior: if fully transparent, return a 1x1 transparent image.
    if x_min == 0 and x_max == 0 and y_min == 0 and y_max == 0:
        return Image.new("RGBA", (1, 1), (0, 0, 0, 0)), 0, 0
    img = img_base.crop((x_min, y_min, x_max + 1, y_max + 1))
    return img, x_min, y_min  # img + pos on img
    
            
            
def convert_to_only_Alpha(img_base, x_size, y_size, respect_ratio = False, rotate_90 = False, miror = False
                          , new_ratio = 0, cut = [[0, 0],[0, 0]], add_SHARPEN = False):
    # Fast path: for segments we only need alpha, so transform in L mode.
    alpha_img, x_size, y_size = transform_alpha_mask(
        img_base,
        x_size,
        y_size,
        respect_ratio,
        rotate_90,
        miror,
        new_ratio,
        cut,
        add_SHARPEN,
    )

    bbox = alpha_img.getbbox()
    if bbox is None:
        alpha_img = Image.new("L", (1, 1), 0)
        pos_x, pos_y = 0, 0
    else:
        left, upper, right, lower = bbox
        pos_x, pos_y = left, upper
        alpha_img = alpha_img.crop(bbox)

    out = Image.new("RGBA", alpha_img.size, (255, 255, 255, 0))
    out.putalpha(alpha_img)
    return out, pos_x, pos_y, x_size, y_size


def transform_alpha_mask(image_b, x_size, y_size, respect_ratio, rotate_90, miror,
                         new_ratio=0, cut=[[0, 0], [0, 0]], add_SHARPEN=False):
    """Transform an image but keep only its alpha channel (L mode).

    This is much faster than doing the same work in RGBA when we only need the
    final alpha mask (segments).
    """

    alpha = image_b.convert("RGBA").getchannel("A")
    alpha = alpha.point(_ALPHA_LUT_30)

    w, h = alpha.size

    # Calcul cuts en pixels
    x_cut_start = int(cut[0][0] * w)
    x_cut_end = int(cut[0][1] * w)
    if new_ratio != 0:
        y_cut_start = int(cut[1][0] * w / new_ratio)
        y_cut_end = int(cut[1][1] * w / new_ratio)
    else:
        y_cut_start = int(cut[1][0] * h)
        y_cut_end = int(cut[1][1] * h)

    x_start = max(0, x_cut_start)
    y_start = max(0, y_cut_start)
    x_end = max(0, x_cut_end)
    y_end = max(0, y_cut_end)

    new_w = w - x_start - x_end
    new_h = h - y_start - y_end

    canvas_w = new_w + max(-x_cut_start, 0) + max(-x_cut_end, 0)
    canvas_h = new_h + max(-y_cut_start, 0) + max(-y_cut_end, 0)
    new_alpha = Image.new("L", (canvas_w, canvas_h), 0)

    paste_x = max(-x_cut_start, 0)
    paste_y = max(-y_cut_start, 0)

    crop_box = (x_start, y_start, x_start + new_w, y_start + new_h)
    cropped = alpha.crop(crop_box)
    new_alpha.paste(cropped, (paste_x, paste_y))
    alpha = new_alpha

    if miror:
        alpha = alpha.transpose(Image.FLIP_LEFT_RIGHT)
    if rotate_90:
        alpha = alpha.rotate(-90, expand=True)
        x_size, y_size = y_size, x_size
        if new_ratio != 0:
            new_ratio = 1 / new_ratio

    if new_ratio != 0:
        y_size = min(y_size, int(x_size / new_ratio))
        x_size = min(x_size, int(y_size * new_ratio))
    elif respect_ratio:
        ratio = alpha.size[0] / alpha.size[1]
        y_size = min(y_size, int(x_size / ratio))
        x_size = min(x_size, int(y_size * ratio))

    alpha = alpha.resize((x_size, y_size), resample=Image.LANCZOS)
    if add_SHARPEN:
        alpha = alpha.filter(ImageFilter.UnsharpMask(radius=1, percent=50, threshold=3))

    alpha = alpha.point(_ALPHA_LUT_30)
    return alpha, x_size, y_size
            
                       
 
def make_alpha(img_base:np.array, fond_bright:float, alpha_bright:float):
    img_alpha = ImageEnhance.Brightness(img_base.copy())
    img_alpha = img_alpha.enhance(alpha_bright)
    
    img = ImageEnhance.Brightness(img_base.copy())
    img = img.enhance(fond_bright)

    data = np.array(img)
    data_alpha = np.array(img_alpha)
    r, g, b, a = data_alpha[:,:,0], data_alpha[:,:,1], data_alpha[:,:,2], data_alpha[:,:,3]
    transition = np.zeros((len(r), len(r[0])), dtype=np.float64)
    for x in range(len(r)):
        for y in range(len(r[0])):
            if ((r[x, y]>seuil_tr) and (g[x, y]>seuil_tr) and (b[x, y]>seuil_tr)): transition[x, y] = min(0, a[x, y])
            elif (min(min(r[x, y], g[x, y]), b[x, y]) < seuil_semi_tr): transition[x, y] = min(255, a[x, y])
            else: transition[x, y] = min(int(255.0 * float(seuil_tr - min(min(r[x, y], g[x, y]), b[x, y])) / float(seuil_tr - seuil_semi_tr)), float(a[x, y])) 

    data[:,:, 3] = transition
    return data


def ratio_cut(transform, i):
    if(transform != []): # calculate if need to cut and change ratio of image segments
        new_ratio = (transform[i][0][0]-transform[i][0][1]-transform[i][0][2]) / (transform[i][1][0]-transform[i][1][1]-transform[i][1][2])
        x_cut = [transform[i][0][1]/transform[i][0][0], transform[i][0][2]/transform[i][0][0]]
        y_cut = [transform[i][1][1]/transform[i][1][0], transform[i][1][2]/transform[i][1][0]]
    else :
        new_ratio = 0
        x_cut = [0, 0]
        y_cut = [0, 0]
    return new_ratio, x_cut, y_cut
            


def save_packed_img(packer, all_img: list, atlas_size: list, pad: int, destination_graphique_file: str, name: str):
    # optimise in one texture
    packer.add_bin(atlas_size[0], atlas_size[1])
    packer.pack()
    atlas = Image.new("RGBA", atlas_size, (0, 0, 0, 0))

    # Speed up lookup: filename -> record
    by_name = {rec[0]: rec for rec in all_img}
    
    for img_ in packer[0].rect_list():
        x, y, w, h, img_index = img_
        rec = by_name.get(img_index)
        if rec is None:
            continue
        atlas.paste(rec[1], (x + pad, y + pad))
        rec.append(x + pad)
        rec.append(atlas_size[1] - y - h + pad) # in 3ds, y is on button not on up
        
    atlas.save(destination_graphique_file + 'segment_' + name + '.png')
