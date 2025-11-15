from PIL import Image, ImageEnhance, ImageFilter
import numpy as np

seuil_tr = 170 # transparency
seuil_semi_tr = 100 # semi-transparency



################ Convert PNG to Emulator ############################################
def transform_img(image_b, x_size, y_size, respect_ratio, rotate_90, miror
                  , new_ratio = 0, cut = [[0, 0],[0, 0]], add_SHARPEN=False):
    image_base = image_b.convert("RGBA")

    # delete low alpha
    arr = np.array(image_base)
    alpha = arr[:, :, 3]
    alpha[alpha < 30] = 0
    arr[:, :, 3] = alpha
    image_base = Image.fromarray(arr, mode="RGBA")

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

    # Reapply alpha recreate by resize img
    arr = np.array(img)
    alpha = arr[:, :, 3]
    alpha[alpha < 30] = 0
    arr[:, :, 3] = alpha
    img = Image.fromarray(arr, mode="RGBA")

    return img, x_size, y_size

    
    
def find_x_y_size(img_base):
    img = img_base.copy()
    img = np.array(img)
    alpha = img[:, :, 3]
    coords = np.argwhere(alpha > 0)
    if coords.size == 0:
        return 0,0,0,0
    y_min, x_min = coords.min(axis=0)
    y_max, x_max = coords.max(axis=0)
    return x_min, x_max, y_min, y_max


def crop_img(img_base):
    x_min, x_max, y_min, y_max = find_x_y_size(img_base)
    img = img_base.crop((x_min, y_min, x_max+1, y_max+1))
    return img, x_min, y_min # img + pos on img
    
            
            
def convert_to_only_Alpha(img_base, x_size, y_size, respect_ratio = False, rotate_90 = False, miror = False
                          , new_ratio = 0, cut = [[0, 0],[0, 0]], add_SHARPEN = False):
    img, x_size, y_size = transform_img(img_base, x_size, y_size, respect_ratio
                                        , rotate_90, miror, new_ratio, cut, add_SHARPEN)
    img, pos_x, pos_y = crop_img(img)
    img = np.array(img)
    
    img_no_color = np.array(img)
    img_no_color = np.zeros_like(img_no_color)
    img_no_color[..., 0] = 255 
    img_no_color[..., 1] = 255          
    img_no_color[..., 2] = 255          
    img_no_color[..., 3] = img[..., 3]
    img_no_color = Image.fromarray(img_no_color.astype(np.uint8), mode="RGBA")
    return img_no_color, pos_x, pos_y, x_size, y_size
            
                       
 
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
    
    for img_ in packer[0].rect_list():
        curr_i = 0
        x, y, w, h, img_index = img_
        while(curr_i < len(all_img) and img_index != all_img[curr_i][0]):
            x, y, w, h, img_index = img_
            curr_i = curr_i+1
        atlas.paste(all_img[curr_i][1], (x+pad, y+pad))
        all_img[curr_i].append(x+pad)
        all_img[curr_i].append(atlas_size[1]-y-h+pad) # in 3ds, y is on button not on up
        
    atlas.save(destination_graphique_file + 'segment_' + name + '.png')
