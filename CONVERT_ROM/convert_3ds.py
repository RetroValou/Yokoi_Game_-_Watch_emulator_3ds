import os
import shutil
from PIL import Image, ImageEnhance
import numpy as np
from rectpack import newPacker
from multiprocessing import Pool
import functools

from source import convert_svg as cs
from source import img_manipulation as im



INKSCAPE_PATH = r"C:\Program Files\Inkscape\bin\inkscape.exe"

destination_file = r"..\source\std\GW_ALL.h"
destination_game_file = r"..\source\std\GW_ROM"
destination_graphique_file = "../gfx/"

reset_img_svg = False
resolution_up = [400, 240]
resolution_down = [320, 240]
demi_resolution_up = [200, 240]


size_altas_check = [256, 512, 128, 32, 64, 1024]
pad = 1
console_atlas_size = [512, 256]
console_size = [320, 240]

default_alpha_bright = 1.7
default_fond_bright = 1.35
default_rotate = False
default_console = r'.\rom\default.png'

from generated_games_path import games_path

games_path__ = {
              "ball" :
                    { "ref" : "ac-01"
                    , "display_name" : "Ball"
                    , "Rom" : r'.\rom\ball\ac-01'
                    , "Visual" : [r'.\rom\ball\gnw_ball.svg'] # list of screen visual
                    , "Background" :[r'.\rom\ball\Background2NS.png']
                    , "transform_visual" : [[[2242, 121, 121], [1449, 69, 88]]]
                    , "date" : "1980-04-28"
                    , "console" : r'.\rom\ball\gnw_ball.png'
                }
              
            , "Flagman" :
                    { "ref" : "fl-02"
                    , "display_name" : "Flagman"
                    , "Rom" : r'.\rom\gnw_flagman\fl-02'
                    , "Visual" : [r'.\rom\gnw_flagman\gnw_flagman.svg'] # list of screen visual
                    #, "transform_visual" : [[[1266, 28, 18], [835, 8, 32]]]}
                    , "transform_visual" : [[[1266, 28, 18], [900, 8, 32]]]
                    , "date" : "1980-06-05"
                    , "console" : r'.\rom\gnw_flagman\gnw_flagman.png'
                }
                              
            , "Vermin" :
                    { "ref" : "mt-03"
                    , "display_name" : "Vermin"
                    , "Rom" : r'.\rom\gnw_vermin\mt-03'
                    , "Visual" : [r'.\rom\gnw_vermin\gnw_vermin.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_vermin\Background2NS.png']
                    , "transform_visual" : [[[2329, 145, 184], [1505, 88, 125]]]
                    , "date" : "1980-07-10"
                    , "console" : r'.\rom\gnw_vermin\gnw_vermin.png'
                    }
              
            , "Fire" :
                    { "ref" : "rc-04"
                    , "display_name" : "Fire"
                    , "Rom" : r'.\rom\gnw_fires\rc-04'
                    , "Visual" : [r'.\rom\gnw_fires\gnw_fires.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_fires\Background2NS.png']
                    , "transform_visual" : [[[2300, 150, 150], [1510, 125, 93]]]
                    , "date" : "1980-07-31"
                    , "console" : r'.\rom\gnw_fires\gnw_fires.png'
                    }
              
            , "Judge" :
                    { "ref" : "ip-05"
                    , "display_name" : "Judge"
                    , "Rom" : r'.\rom\gnw_judge\ip-05'
                    , "Visual" : [r'.\rom\gnw_judge\gnw_judge.svg'] # list of screen visual
                    , "transform_visual" : [[[2359, 176, 183], [1548, 90, 166]]]
                    , "date" : "1980-10-04"
                    , "console" : r'.\rom\gnw_judge\gnw_judge.png'
                    }
                      
            , "Manhole" :
                    { "ref" : "MH_06"
                    , "display_name" : "Manhole"
                    , "Rom" : r'.\rom\gnw_manholeg\mh-06'
                    , "Visual" : [r'.\rom\gnw_manholeg\gnw_manholeg.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_manholeg\BackgroundNSM.png']
                    , "transform_visual" : [[[2396, 200, 160], [1607, 140, 130]]]
                    , "date" : "1981-01-27"
                    , "console" : r'.\rom\gnw_manholeg\gnw_manholeg.png'
                    }
              
            , "Helmet" :
                    { "ref" : "cn-17"
                    , "display_name" : "Helmet"
                    , "Rom" : r'.\rom\gnw_helmet\cn-17'
                    , "Visual" : [r'.\rom\gnw_helmet\gnw_helmet.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_helmet\BackgroundNS.png']
                    , "transform_visual" : [[[1145, 86, 101], [747, 58, 66]]]
                    , "date" : "1981-02-21"
                    , "console" : r'.\rom\gnw_helmet\gnw_helmet.png'
                    }
              
            , "Lion" :
                    { "ref" : "LN_08"
                    , "display_name" : "Lion"
                    , "Rom" : r'.\rom\gnw_lion\ln-08'
                    , "Visual" : [r'.\rom\gnw_lion\gnw_lion.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_lion\BackgroundNS.png']
                    , "transform_visual" : [[[2361, 146, 179], [1627, 145, 145]]]
                    , "date" : "1981-04-27"
                    , "console" : r'.\rom\gnw_lion\gnw_lion.png'
                    }
              
            , "Parachute" :
                    { "ref" : "pr-21"
                    , "display_name" : "Parachute"
                    , "Rom" : r'.\rom\gnw_pchute\pr-21'
                    , "Visual" : [r'.\rom\gnw_pchute\gnw_pchute.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_pchute\BackgroundNS.png']
                    , "transform_visual" : [[[1026, 25, 38], [699, 44, 40]]]
                    , "date" : "1981-06-16"
                    , "console" : r'.\rom\gnw_pchute\gnw_pchute.png'
                    }
              
            , "octopus" :
                    { "ref" : "OC_22"
                    , "display_name" : "Octopus"
                    , "Rom" : r'.\rom\gnw_octopus\oc-22'
                    , "Visual" : [r'.\rom\gnw_octopus\gnw_octopus.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_octopus\BackgroundNS.png']
                    , "transform_visual" : [[[1026, 25, 38], [699, 44, 40]]]
                    , "date" : "1981-07-16"
                    , "console" : r'.\rom\gnw_octopus\gnw_octopus.png'
                    }
              
             , "Popeye" :
                    { "ref" : "PP-23"
                    , "display_name" : "Popeye"
                    , "Rom" : r'.\rom\gnw_popeye\pp-23'
                    , "Visual" : [r'.\rom\gnw_popeye\gnw_popeye.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_popeye\BackgroundNS.png']
                    , "transform_visual" : [[[1317, 46, 48], [904, 60, 62]]]
                    , "date" : "1981-08-05"
                    , "console" : r'.\rom\gnw_popeye\gnw_popeye.png'
                    }
              
            , "Chef" :
                    { "ref" : "fp-24"
                    , "display_name" : "Chef"
                    , "Rom" : r'.\rom\gnw_chef\fp-24'
                    , "Visual" : [r'.\rom\gnw_chef\gnw_chef.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_chef\BackgroundNS.png']
                    , "transform_visual" : [[[1282, 17, 14], [831, 23, 9]]]
                    , "date" : "1981-09-08"
                    , "console" : r'.\rom\gnw_chef\gnw_chef.png'
                    }
              
             , "Mickey_Mouse" :
                    { "ref" : "MC_25"
                    , "display_name" : "Mickey Mouse"
                    , "Rom" : r'.\rom\gnw_mmouse\mc-25'
                    , "Visual" : [r'.\rom\gnw_mmouse\gnw_mmouse.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_mmouse\BackgroundNS.png']
                    , "transform_visual" : [[[1405, 70, 90], [906, 60, 50]]]
                    , "date" : "1981-10-09"
                    , "console" : r'.\rom\gnw_mmouse\gnw_mmouse.png'
                    }
              
             , "Egg" :
                    { "ref" : "EG_26"
                    , "display_name" : "Egg"
                    , "Rom" : r'.\rom\gnw_egg\mc-25'
                    , "Visual" : [r'.\rom\gnw_egg\gnw_egg.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_egg\BackgroundNS.png']
                    , "transform_visual" : [[[1415, 75, 95], [901, 55, 50]]]
                    , "date" : "1981-10-09"
                    , "console" : r'.\rom\gnw_egg\gnw_egg.png'
                    }
              
            , "Fire_wide_screen" :
                    { "ref" : "fr-27"
                    , "display_name" : "Fire"
                    , "Rom" : r'.\rom\gnw_fire\fr-27'
                    , "Visual" : [r'.\rom\gnw_fire\gnw_fire.svg'] # list of screen visual
                    , "Background" : [r'.\rom\gnw_fire\BackgroundNS.png']
                    , "transform_visual" : [[[1315, 26, 44], [875, 40, 39]]]
                    , "date" : "1981-12-04"
                    , "console" : r'.\rom\gnw_fire\gnw_fire.png'
                    }
              
            , "Turtle_Bridge" :
                    { "ref" : "tl-28"
                    , "display_name" : "Turtle Bridge"
                    , "Rom" : r'.\rom\gnw_tbridge\tl-28'
                    , "Visual" : [r'.\rom\gnw_tbridge\gnw_tbridge.svg'] # list of screen visual
                    , "Background" : [r'.\rom\gnw_tbridge\BackgroundNS.png']
                    , "alpha_bright" : 1.2
                    , "fond_bright" : 1.3
                    , "transform_visual" : [[[1286, 22, 51], [875, 53, 20]]]
                    , "date" : "1982-02-01"
                    , "console" : r'.\rom\gnw_tbridge\gnw_tbridge.png'
                    }
              
            , "Fire_Attack" :
                    { "ref" : "id-29"
                    , "display_name" : "Fire Attack"
                    , "Rom" : r'.\rom\gnw_fireatk\id-29'
                    , "Visual" : [r'.\rom\gnw_fireatk\gnw_fireatk.svg'] # list of screen visual
                    , "Background" : [r'.\rom\gnw_fireatk\BackgroundNS.png']
                    , "alpha_bright" : 1.1
                    , "fond_bright" : 1.2
                    , "shadow" : False
                    , "transform_visual" : [[[1350, 62, 67], [885, 50, 54]]]
                    , "date" : "1982-03-26"
                    , "console" : r'.\rom\gnw_fireatk\gnw_fireatk.png'
                    }
              
            , "Snoopy_Tennis" :
                    { "ref" : "sp-30"
                    , "display_name" : "Snoopy Tennis"
                    , "Rom" : r'.\rom\gnw_stennis\sp-30'
                    , "Visual" : [r'.\rom\gnw_stennis\gnw_stennis.svg'] # list of screen visual
                    , "Background" : [r'.\rom\gnw_stennis\BackgroundNS.png']
                    , "transform_visual" : [[[1294, 42, 39], [883, 49, 63]]]
                    , "date" : "1982-04-28"
                    , "console" : r'.\rom\gnw_stennis\gnw_stennis.png'
                }

            ,"Oil_Panic" :
                    { "ref" : "op-51"
                    , "display_name" : "Oil Panic"
                    , "date" : "1982-05-28"
                    , "Rom" : r'.\rom\gnw_opanic\op-51'
                    , "Visual" : [r'.\rom\gnw_opanic\gnw_opanic_top.svg'
                                    , r'.\rom\gnw_opanic\gnw_opanic_bottom.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_opanic\Screen-TopNS.png'
                                    , r'.\rom\gnw_opanic\Screen-BottomNS.png']
                    , "size_visual" : [resolution_up, [330, 240]]
                    , "alpha_bright" : 1.2
                    , "fond_bright" : 1.3
                    , "transform_visual" : [[[1349, 33, 22], [899, 24, 58]], [[1371, 46, 51], [878, -18, 79]]]
                    , "console" : r'.\rom\gnw_opanic\gnw_opanic.png'
                }
              
            ,"Donkey_kong" :
                    { "ref" : "dk-52"
                    , "display_name" : "Donkey Kong"
                    , "date" : "1982-06-03"
                    , "Rom" : r'.\rom\gnw_dkong\dk-52'
                    , "Visual" : [r'.\rom\gnw_dkong\gnw_dkong_top.svg'
                                    , r'.\rom\gnw_dkong\gnw_dkong_bottom.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_dkong\Screen-TopNS.png'
                                    , r'.\rom\gnw_dkong\Screen-BottomNS.png']
                    , "size_visual" : [[330, 240], [330, 240]]
                    , "transform_visual" : [[[1339, 16, 27], [870, 18, 35]], [[1319, 8, 15], [854, 11, 26]]]
                    , "console" : r'.\rom\gnw_dkong\gnw_dkong.png'
                }

            , "Donkey_Kong_JR" :
                    { "ref" : "dj-101"
                    , "display_name" : "Donkey Kong JR"
                    , "Rom" : r'.\rom\gnw_dkjr\dj-101'
                    , "Visual" : [r'.\rom\gnw_dkjr\gnw_dkjr.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_dkjr\BackgroundNS.png']
                    , "transform_visual" : [[[1216, 1, 6], [798, 5, -5]]]
                    , "date" : "1982-10-26"
                    , "console" : r'.\rom\gnw_dkjr\gnw_dkjr.png'
                    }
              
            ,"Mickey_Donald" :
                    { "ref" : "dm-53"
                    , "display_name" : "Mickey Donald"
                    , "date" : "1982-11-12"
                    , "Rom" : r'.\rom\gnw_mickdon\dm-53_565'
                    , "Visual" : [r'.\rom\gnw_mickdon\gnw_mickdon_top.svg'
                                    , r'.\rom\gnw_mickdon\gnw_mickdon_bottom.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_mickdon\Screen-TopNS.png'
                                    , r'.\rom\gnw_mickdon\Screen-BottomNS.png']
                    , "size_visual" : [[350, 240], [350, 240]]
                    , "transform_visual" : [[[1288, -8, 0], [844, 1, 26]], [[1273, -24, 1], [802, 13, -28]]]
                    , "console" : r'.\rom\gnw_mickdon\gnw_mickdon.png'
                }
              
            ,"Green_House" :
                    { "ref" : "GH_54"
                    , "display_name" : "Green House"
                    , "date" : "1982-12-06"
                    , "Rom" : r'.\rom\gnw_ghouse\gh-54'
                    , "Visual" : [r'.\rom\gnw_ghouse\gnw_ghouse_top.svg'
                                    , r'.\rom\gnw_ghouse\gnw_ghouse_bottom.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_ghouse\Screen-TopNS.png'
                                    , r'.\rom\gnw_ghouse\Screen-BottomNS.png']
                    , "size_visual" : [[360, 240], [360, 240]]
                    , "transform_visual" : [[[1330, 16+6, 18+6], [886, 28, 41]], [[1329, 16, 17], [864, 14, 33]]]
                    , "console" : r'.\rom\gnw_ghouse\gnw_ghouse.png'
                }

            ,"Donkey_Kong_2" :
                    { "ref" : "jr-55"
                    , "display_name" : "Donkey Kong 2"
                    , "date" : "1983-03-07"
                    , "Rom" : r'.\rom\gnw_dkong2\jr-55_560'
                    , "Visual" : [r'.\rom\gnw_dkong2\gnw_dkong2_top.svg'
                                    , r'.\rom\gnw_dkong2\gnw_dkong2_bottom.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_dkong2\Screen-TopNS.png'
                                    , r'.\rom\gnw_dkong2\Screen-BottomNS.png']
                    , "size_visual" : [[344, 240], [344, 240]]
                    , "transform_visual" : [[[1319, 0, 23], [827, 15, -5]], [[1314, 8, 10], [825, 3, 5]]]
                    , "console" : r'.\rom\gnw_dkong2\gnw_dkong2.png'
                }
              
            ,"Mario_Bros" :
                    { "ref" : "mw-56"
                    , "display_name" : "Mario Bros"
                    , "date" : "1983-03-14"
                    , "Rom" : r'.\rom\gnw_mario\mw-56'
                    , "Visual" : [r'.\rom\gnw_mario\rework\gnw_mario_left.svg'
                                    , r'.\rom\gnw_mario\rework\gnw_mario_right.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_mario\rework\Screen-LeftNS.png'
                                    , r'.\rom\gnw_mario\rework\Screen-RightNS.png']
                    , "size_visual" : [[234, 240], [234, 240]]
                    , "2_in_one_screen" : True
                    , "shadow" : False
                    , "transform_visual" : [[[1248, 17, 17], [782, 6, 0]], [[1217, 1, 2], [785, 3+10, 6-10]]]
                    , "console" : r'.\rom\gnw_mario\Backdrop_2.png'
                }
              
            , "Mario_Cement_Factory_panorama" :
                    { "ref" : "cm-72"
                    , "display_name" : "Mario Cement Factory"
                    , "date" : "1983-04-28"
                    , "Rom" : r'.\rom\gnw_mariocmt\cm-72.program'
                    , "Visual" : [r'.\rom\gnw_mariocmt\gnw_mariocmt.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_mariocmt\cm-72.melody'
                    , "Background" : [r'.\rom\gnw_mariocmt\background2.png']
                    , "mask" : True
                    , "transform_visual" : [[[2074, -74, -80], [1240, -56, -80]]]
                    , "console" : r'.\rom\gnw_mariocmt\gnw_mariocmt.png'
                }
              
            , "Mario_Cement_Factory" :
                    { "ref" : "ml-102"
                    , "display_name" : "Mario Cement Factory"
                    , "Rom" : r'.\rom\gnw_mariocm\ml-102_577'
                    , "Visual" : [r'.\rom\gnw_mariocm\gnw_mariocm.svg'] # list of screen visual
                    , "Background" : [r'.\rom\gnw_mariocm\BackgroundNS.png']
                    , "transform_visual" : [[[1227, 8, 3], [816, 25, -7]]]
                    , "date" : "1983-06-16"
                    , "console" : r'.\rom\gnw_mariocm\gnw_mariocm.png'
                }

            ,"Rain_Shower" :
                    { "ref" : "lp-57"
                    , "display_name" : "Rain Shower"
                    , "date" : "1983-08-10"
                    , "Rom" : r'.\rom\gnw_rshower\lp-57'
                    , "Visual" : [r'.\rom\gnw_rshower\gnw_rshower_left.svg'
                                    , r'.\rom\gnw_rshower\gnw_rshower_right.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_rshower\Screen-LeftNS.png'
                                    , r'.\rom\gnw_rshower\Screen-RightNS.png']
                    , "size_visual" : [[200, 240], [200, 240]]
                    , "2_in_one_screen" : True
                    , "shadow" : False
                    , "alpha_bright" : 1.3
                    , "fond_bright" : 1.3
                    , "transform_visual" : [[[1280, 19, 47], [855, 47, 19]], [[1300, 48, 38], [853, 65, 12]]]
                    , "console" : r'.\rom\gnw_rshower\Backdrop_2.png'
                }
              
            , "Manhole_wide_screen" :
                    { "ref" : "NH_103"
                    , "display_name" : "Manhole"
                    , "Rom" : r'.\rom\gnw_manhole\nh-103'
                    , "Visual" : [r'.\rom\gnw_manhole\gnw_manhole.svg'] # list of screen visual
                    , "Background" : [r'.\rom\gnw_manhole\BackgroundNS.png']
                    , "transform_visual" : [[[1290, 37, 36], [900, 56, 45]]]
                    , "date" : "1983-08-24"
                    , "console" : r'.\rom\gnw_manhole\gnw_manhole.png'
                }
              
            , "Snoopy_Table_Top" :
                    { "ref" : "sm-91"
                    , "display_name" : "Snoopy"
                    , "date" : "1983-08-30"
                    , "Rom" : r'.\rom\gnw_snoopyp\sm-91.program'
                    , "Visual" : [r'.\rom\gnw_snoopyp\gnw_snoopyp.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_snoopyp\sm-91.melody'
                    , "Background" : [r'.\rom\gnw_snoopyp\Background.png']
                    , "mask" : True
                    , "transform_visual" : [[[1328, 0, 0], [703, -9, -9]]]
                    , "console" : r'.\rom\gnw_snoopyp\gnw_snoopyp.png'
                }

            , "Popeye_Table_Top" :
                    { "ref" : "PG_92"
                    , "display_name" : "Popeye"
                    , "date" : "1983-08-30"
                    , "Rom" : r'.\rom\gnw_popeyep\pg-92.program'
                    , "Visual" : [r'.\rom\gnw_popeyep\gnw_popeyep.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_popeyep\pg-92.melody'
                    , "Background" : [r'.\rom\gnw_popeyep\Background.png']
                    , "mask" : True
                    , "transform_visual" : [[[1328, 0, 0], [721, 0, 0]]]
                    , "console" : r'.\rom\gnw_popeyep\gnw_popeyep.png'
            }
              
            , "Donkey_Kong_Circus" :
                    { "ref" : "dc-95"
                    , "display_name" : "Donkey Kong Circus"
                    , "date" : "1983-09-06"
                    , "Rom" : r'.\rom\gnw_mmousep\dc-95.program'
                    , "Visual" : [r'.\rom\gnw_dkcirc\gnw_dkcirc.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_mmousep\dc-95.melody'
                    , "Background" : [r'.\rom\gnw_dkcirc\Background.png']
                    , "mask" : True
                    , "transform_visual" : [[[1620, 42, 25], [880, 37, 34]]]
                    , "console" : r'.\rom\gnw_dkcirc\gnw_dkcirc.png'
                }
              
            , "DK_JR_panorama" :
                    { "ref" : "cj-93"
                    , "display_name" : "Donkey Kong JR"
                    , "date" : "1983-10-07"
                    , "Rom" : r'.\rom\gnw_dkjrp\cj-93.program'
                    , "Visual" : [r'.\rom\gnw_dkjrp\gnw_dkjrp.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_dkjrp\cj-93.melody'
                    , "Background" : [r'.\rom\gnw_dkjrp\Background.png']
                    , "mask" : True
                    , "transform_visual" : [[[1650, 0, 0], [886, 0, 0]]]
                    , "console" : r'.\rom\gnw_dkjrp\gnw_dkjrp.png'
                }
              
              
            ,"Life_Boat" :
                    { "ref" : "tc-58"
                    , "display_name" : "Life Boat"
                    , "date" : "1983-10-25"
                    , "Rom" : r'.\rom\gnw_lboat\tc-58'
                    , "Visual" : [r'.\rom\gnw_lboat\rework\gnw_lboat_left.svg'
                                    , r'.\rom\gnw_lboat\rework\gnw_lboat_right.svg'] # list of screen visual
                    , "Background" :[r'.\rom\gnw_lboat\rework\Screen-LeftNS.png'
                                    , r'.\rom\gnw_lboat\rework\Screen-RightNS.png']
                    , "size_visual" : [[234, 240], [234, 240]]
                    , "2_in_one_screen" : True
                    , "shadow" : False
                    , "alpha_bright" : 1.2
                    , "transform_visual" : [[[1244, 18, 12], [836, 39, 21]], [[1273, 32, 27], [882, 86, 20]]]
                    , "console" : r'.\rom\gnw_lboat\Backdrop_2.png'
                }
              
            , "Mario_Bombs_Away" :
                    { "ref" : "tb-94"
                    , "display_name" : "Mario Bombs Away"
                    , "date" : "1983-11-10"
                    , "Rom" : r'.\rom\gnw_mbaway\tb-94.program'
                    , "Visual" : [r'.\rom\gnw_mbaway\gnw_mbaway.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_mbaway\tb-94.melody'
                    , "Background" : [r'.\rom\gnw_mbaway\background.png']
                    , "mask" : True
                    , "transform_visual" : [[[1650, 0, 0], [886, 0, 0]]]
                    , "console" : r'.\rom\gnw_mbaway\gnw_mbaway.png'
                }
              
            ,"Pinball" :
                    { "ref" : "PB_59"
                    , "display_name" : "Pinball"
                    , "date" : "1983-12-02"
                    , "Rom" : r'.\rom\gnw_pinball\pb-59.program'
                    , "Visual" : [r'.\rom\gnw_pinball\gnw_pinball_top.svg'
                                    , r'.\rom\gnw_pinball\gnw_pinball_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_pinball\pb-59.melody'
                    , "Background" : [r'.\rom\gnw_pinball\Screen-TopNS.png'
                                    , r'.\rom\gnw_pinball\Screen-BottomNS.png']
                    , "size_visual" : [[326, 240], [326, 240]]
                    , "transform_visual" : [[[1425, 64, 65], [928, 31, 80]], [[1438, 71, 71], [955, 31, 108]]]
                    , "console" : r'.\rom\gnw_pinball\gnw_pinball.png'
                }
              
            , "Mickey_Mouse_panorama" :
                    { "ref" : "dc-95"
                    , "display_name" : "Mickey Mouse"
                    , "date" : "1984-02-XX"
                    , "Rom" : r'.\rom\gnw_mmousep\dc-95.program'
                    , "Visual" : [r'.\rom\gnw_mmousep\gnw_mmousep.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_mmousep\dc-95.melody'
                    , "Background" : [r'.\rom\gnw_mmousep\Backgroundnew.png']
                    , "mask" : True
                    , "transform_visual" : [[[1701, 34, 41], [984, 50, 22]]]
                    , "console" : r'.\rom\gnw_mmousep\gnw_mmousep.png'
                }
              
            , "crab_grab" :
                    { "ref" : "ud-202"
                    , "display_name" : "Crab Grab"
                    , "date" : "1984-02-07"
                    , "Rom" : r'.\rom\gnw_cgrab\ud-202'
                    , "Visual" : [r'.\rom\gnw_cgrab\rework\gnw_cgrab_up.svg'
                                  , r'.\rom\gnw_cgrab\rework\gnw_cgrab_down.svg'] # up = 48% / down = 52%
                    , "Background" :[r'.\rom\gnw_cgrab\rework\background_up.png'
                                     , r'.\rom\gnw_cgrab\rework\background_down.png']
                    , "size_visual" : [resolution_down, resolution_down]
                    , "color_segment" : True
                    , "shadow" : False
                    , "alpha_bright" : 1
                    , "fond_bright" : 1.3
                    , "transform_visual" : [[[1003, 31, 31], [819-3, -26+3, 0]], [[1003, 31, 31], [906-20-22-4, 4, 22]]]
                    , "console" : r'.\rom\gnw_cgrab\gnw_cgrab.png'
            }
              
            , "Spitball_Sparky" :
                    { "ref" : "bu-201"
                    , "display_name" : "Spitball Sparky"
                    , "date" : "1984-02-21"
                    , "Rom" : r'.\rom\gnw_ssparky\bu-201'
                    , "Visual" : [r'.\rom\gnw_ssparky\rework\gnw_ssparky_up.svg'
                                    , r'.\rom\gnw_ssparky\rework\gnw_ssparky_down.svg'] # up = 48% / down = 52%
                    , "Background" :[r'.\rom\gnw_ssparky\rework\background_up.png'
                                    , r'.\rom\gnw_ssparky\rework\background_down.png']
                    , "size_visual" : [resolution_down, resolution_down]
                    , "color_segment" : True
                    , "shadow" : False
                    , "alpha_bright" : 1
                    , "fond_bright" : 1.3
                    , "transform_visual" : [[[1003, -32, -34], [864-10, -16+10, 0]], [[1003, -32, -34], [864-10, 0, -17+10]]]
                    , "console" : r'.\rom\gnw_ssparky\gnw_ssparky.png'
            }
              
             , "boxing" :
                    { "ref" : "BX_301"
                    , "display_name" : "Boxing"
                    , "date" : "1984-07-31"
                    , "Rom" : r'.\rom\gnw_boxing\bx-301_744.program'
                    , "Visual" : [r'.\rom\gnw_boxing\gnw_boxing.svg']
                    , "Melody_Rom" : r'.\rom\gnw_boxing\bx-301_744.melody'
                    , "Background" :[r'.\rom\gnw_boxing\BackgroundNS.png']
                    , "transform_visual" : [[[2291, -33, -23], [663, -36, -31]]]
                    , "console" : r'.\rom\gnw_boxing\gnw_boxing.png'
            }
              
            , "Donkey_Kong_3" :
                    { "ref" : "ak-302"
                    , "display_name" : "Donkey Kong 3"
                    , "date" : "1984-08-20"
                    , "Rom" : r'.\rom\gnw_dkong3\ak-302.program'
                    , "Visual" : [r'.\rom\gnw_dkong3\gnw_dkong3.svg']
                    , "Melody_Rom" : r'.\rom\gnw_dkong3\ak-302.melody'
                    , "Background" :[r'.\rom\gnw_dkong3\BackgroundNS.png']
                    , "transform_visual" : [[[2400, 36, 17], [724, -22, 16]]]
                    , "console" : r'.\rom\gnw_dkong3\gnw_dkong3.png'
            }
            
            , "Donkey_Kong_hockey" :
                    { "ref" : "hk-303"
                    , "display_name" : "Donkey Kong hockey"
                    , "date" : "1984-11-13"
                    , "Rom" : r'.\rom\gnw_dkhockey\hk-303.program'
                    , "Visual" : [r'.\rom\gnw_dkhockey\gnw_dkhockey.svg']
                    , "Melody_Rom" : r'.\rom\gnw_dkhockey\hk-303.melody'
                    , "Background" :[r'.\rom\gnw_dkhockey\BackgroundNS.png']
                    , "transform_visual" : [[[2460, 65, 48], [800, 40, 30]]]
                    , "console" : r'.\rom\gnw_dkhockey\gnw_dkhockey.png'
                }
                           
            ,"black_jack" :
                    { "ref" : "BJ-60"
                    , "display_name" : "Black Jack"
                    , "date" : "1985-02-15"
                    , "Rom" : r'.\rom\gnw_bjack\BJ-60.program'
                    , "Visual" : [r'.\rom\gnw_bjack\gnw_bjack_top.svg'
                                    , r'.\rom\gnw_bjack\gnw_bjack_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_bjack\BJ-60.melody'
                    , "Background" :[r'.\rom\gnw_bjack\Screen-TopNS.png'
                                    , r'.\rom\gnw_bjack\Screen-BottomNS.png']
                    , "transform_visual" : [[[1355, 33, 26], [898, 33, 48]], [[1346, 18, 32], [891, 4, 70]]]
                    , "size_visual" : [ resolution_up, [336, 240]]
                    , "console" : r'.\rom\gnw_bjack\gnw_bjack.png'
                }
              
            , "Tropical_Fish" :
                    { "ref" : "tf-104"
                    , "display_name" : "Tropical Fish"
                    , "Rom" : r'.\rom\gnw_tfish\tf-104'
                    , "Visual" : [r'.\rom\gnw_tfish\gnw_tfish.svg'] # list of screen visual
                    , "Background" : [r'.\rom\gnw_tfish\BackgroundNS.png']
                    , "transform_visual" : [[[1260, 4, 35], [885, 44, 42]]]
                    , "date" : "1985-07-08"
                    , "console" : r'.\rom\gnw_tfish\gnw_tfish.png'
                }
              
            , "Squish" :
                    { "ref" : "mg-61"
                    , "display_name" : "Squish"
                    , "date" : "1986-04-XX"
                    , "Rom" : r'.\rom\gnw_squish\mg-61'
                    , "Visual" : [r'.\rom\gnw_squish\gnw_squish_top.svg'
                                    , r'.\rom\gnw_squish\gnw_squish_bottom.svg'] # list of screen visual
                    , "Background" : [r'.\rom\gnw_squish\Screen-TopNS.png'
                                    , r'.\rom\gnw_squish\Screen-BottomNS.png']
                    , "transform_visual" : [[[1373, 42, 35], [896, 29, 50]], [[1357, 22, 39], [896, 20, 59]]]
                    , "size_visual" : [resolution_up, [360, 240]]
                    , "console" : r'.\rom\gnw_squish\gnw_squish.png'
                }
              
            , "bomb_sweeper" :
                    { "ref" : "bd-62"
                    , "display_name" : "Bomb Sweeper"
                    , "date" : "1987-06-XX"
                    , "Rom" : r'.\rom\gnw_bsweep\bd-62.program'
                    , "Visual" : [r'.\rom\gnw_bsweep\rework\gnw_bsweep_top.svg'
                                    , r'.\rom\gnw_bsweep\rework\gnw_bsweep_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_bsweep\bd-62.melody'
                    , "Background" :[r'.\rom\gnw_bsweep\Screen-TopNS.png'
                                    , r'.\rom\gnw_bsweep\Screen-BottomNS.png']
                    , "transform_visual" : [[[1490, 116, 78], [986, 78, 91]], [[1478, 102, 80], [937, 37, 83]]]
                    , "size_visual" : [ resolution_up, [344, 240]]
                    , "console" : r'.\rom\gnw_bsweep\gnw_bsweep.png'
                }
              
            ,"Safe_Buster" :
                    { "ref" : "jb-63"
                    , "display_name" : "Safe Buster"
                    , "date" : "1988-01-XX"
                    , "Rom" : r'.\rom\gnw_sbuster\jb-63.program'
                    , "Visual" : [r'.\rom\gnw_sbuster\gnw_sbuster_top.svg'
                                    , r'.\rom\gnw_sbuster\gnw_sbuster_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_sbuster\jb-63.melody'
                    , "Background" :[r'.\rom\gnw_sbuster\Screen-TopNS.png'
                                    , r'.\rom\gnw_sbuster\Screen-BottomNS.png']
                    , "size_visual" : [ [330, 240], [340, 240]]
                    , "transform_visual" : [[[1463, 84, 83], [938, 31, 90]], [[1448, 81, 71], [957, 34, 106]]]
                    , "console" : r'.\rom\gnw_sbuster\gnw_sbuster.png'
                }
              
            , "Super_Mario_Bros" :
                    { "ref" : "ym-801"
                    , "display_name" : "Super Mario Bros"
                    , "Rom" : r'.\rom\gnw_smbn\ym-801.program'
                    , "Melody_Rom" : r'.\rom\gnw_smbn\ym-801.melody'
                    , "Visual" : [r'.\rom\gnw_smbn\rework\gnw_smbn.svg'] # list of screen visual
                    , "Background" : [r'.\rom\gnw_smbn\rework\BackgroundNS.png']
                    , "size_visual" : [[400, 260]]
                    , "transform_visual" : [[[1266, 28, 18], [835, 8, 32]]]
                    , "shadow" : False
                    , "alpha_bright" : 1.2
                    , "date" : "1988-03-XX"
                    , "console" : r'.\rom\gnw_smbn\gnw_smbn.png'
                }
              
            , "Ice_Climber" :
                    { "ref" : "dr-802"
                    , "display_name" : "Climber"
                    , "date" : "1988-03-XX"
                    , "Rom" : r'.\rom\gnw_climber\dr-802.program'
                    , "Visual" : [r'.\rom\gnw_climbern\gnw_climbern.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_climber\dr-802.melody'
                    , "Background" : [r'.\rom\gnw_climber\BackgroundNS.png']
                    , "transform_visual" : [[[1321, 52, 44], [861, 27, 30]]]             
                    , "size_visual" : [[400, 250]]
                    , "console" : r'.\rom\gnw_climbern\gnw_climbern.png'
                }
              
            ,"Balloon_Fight" :
                    { "ref" : "BF_107"
                    , "display_name" : "Balloon Fight"
                    , "date" : "1988-03-XX"
                    , "Rom" : r'.\rom\gnw_bfightn\bf-803.program'
                    , "Visual" : [r'.\rom\gnw_bfightn\gnw_bfightn.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_bfightn\bf-803.melody'
                    , "Background" : [r'.\rom\gnw_bfightn\BackgroundNS.png']
                    , "transform_visual" : [[[1281, 25, 32], [917, 56, 60]]]
                    , "console" : r'.\rom\gnw_bfightn\gnw_bfightn.png'
                }
              
            ,"Zelda" :
                    { "ref" : "zl-65"
                    , "display_name" : "Zelda"
                    , "date" : "1988-08-XX"
                    , "Rom" : r'.\rom\gnw_zelda\zl-65.program'
                    , "Visual" : [r'.\rom\gnw_zelda\rework\gnw_zelda_top.svg'
                                    , r'.\rom\gnw_zelda\rework\gnw_zelda_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_zelda\zl-65.melody'
                    , "Background" : [r'.\rom\gnw_zelda\rework\Screen-TopNS.png'
                                    , r'.\rom\gnw_zelda\rework\Screen-BottomNS.png']
                    , "size_visual" : [resolution_up, [340, 240]]
                    , "transform_visual" : [[[1393, 38, 44], [933, 36, 80]], [[1393, 46, 51], [915, 36, 62]]]
                    , "shadow" : True
                    , "console" : r'.\rom\gnw_zelda\gnw_zelda.png'
                }
              
            ,"Gold_Cliff" :
                    { "ref" : "mv-64"
                    , "display_name" : "Gold Cliff"
                    , "date" : "1988-10-XX"
                    , "Rom" : r'.\rom\gnw_gcliff\mv-64.program'
                    , "Visual" : [r'.\rom\gnw_gcliff\gnw_gcliff_top.svg'
                                    , r'.\rom\gnw_gcliff\gnw_gcliff_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_gcliff\mv-64.melody'
                    , "Background" :[r'.\rom\gnw_gcliff\Screen-TopNS.png'
                                    , r'.\rom\gnw_gcliff\Screen-BottomNS.png']
                    , "transform_visual" : [[[1360, 31, 33], [890, 33, 40]], [[1348, 32, 20], [870, 10, 43]]]
                    , "size_visual" : [ [340, 240], [340, 240]]
                    , "console" : r'.\rom\gnw_gcliff\gnw_gcliff.png'
                }
              
            , "Mario_the_Juggle" :
                    { "ref" : "mb-108"
                    , "display_name" : "Mario the Juggler"
                    , "date" : "1991-10-XX"
                    , "Rom" : r'.\rom\gnw_mariotj\mb-108.program'
                    , "Visual" : [r'.\rom\gnw_mariotj\gnw_mariotj.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\gnw_mariotj\mb-108.melody'
                    , "Background" : [r'.\rom\gnw_mariotj\BackgroundNSm.png']
                    , "transform_visual" : [[[1410, 67, 53], [960, 54, 52]]]
                    , "console" : r'.\rom\gnw_mariotj\gnw_mariotj.png'
                }
              
              
}


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
        if(mask): f.write("-f rgba8 -z none\n" + 'segment_' + name + '.png')
        else: f.write("-f a8 -z none\n" + 'segment_' + name + '.png')
    
    result = f'\nconst std::string path_segment_{name} = "romfs:/gfx/segment_{name}.t3x"; // Visual of segment -> Big unique texture'
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
        if(atlas_size[i_a] <= 128): atlas_size[i_a] = 128
        elif(atlas_size[i_a] <= 256): atlas_size[i_a] = 256
        elif(atlas_size[i_a] <= 512): atlas_size[i_a] = 512
        else : atlas_size[i_a] = 1024
        
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
            f.write("-f RGBA8 -z none\n" + 'background_' + name + '.png')
        result = f'\nconst std::string path_background_{name} = "romfs:/gfx/background_{name}.t3x";\n'
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
    target_ratio = console_size[0] / console_size[1]  # 320/240 = 1.333...
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
        f.write("-f RGB8 -z none\n" + 'console_' + name + '.png')
    pos_y = console_atlas_size[1]-img.shape[0]
    result = f'\nconst std::string path_console_{name} = "romfs:/gfx/console_{name}.t3x";\n'
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
    
    cs.extract_group_segs(visual_path, "./tmp/img/"+name, INKSCAPE_PATH)
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
    for key in games_path: c_file_final += f'#include "GW_ROM/{key}.h"\nextern const GW_rom {key};\n'

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
            shutil.rmtree("./tmp/img/" + key)
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
    # Toggle to control processing mode
    USE_PARALLEL_PROCESSING = False  # Set to True to enable multiprocessing

    # Validate all game files before processing
    if not validate_game_files(games_path):
        print("Exiting due to missing files.")
        exit(1)
    
    os.makedirs(r'.\tmp', exist_ok=True)
    os.makedirs(r'.\tmp\img', exist_ok=True)

    # Clean up gfx folder - remove all .t3s and .png files except logo_ and texte_ prefixes
    if os.path.exists(destination_graphique_file):
        for filename in os.listdir(destination_graphique_file):
            if filename.endswith(('.t3s', '.png')):
                if not filename.startswith(('logo_', 'texte_')):
                    file_path = os.path.join(destination_graphique_file, filename)
                    try:
                        os.remove(file_path)
                        print(f"Removed: {filename}")
                    except Exception as e:
                        print(f"Error removing {filename}: {e}")

    # Prepare game data with default values
    game_items = []
    for key in games_path:
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
                print_timing_stats()
                reset_timing_stats()
            except Exception as game_error:
                print(f"Error processing {item[0]}: {game_error}")
                continue

    generate_global_file(games_path, destination_file)
        
    print("\n\n\n\n------------------------------------------------------ Finish !!!!!!!!!!!!")
