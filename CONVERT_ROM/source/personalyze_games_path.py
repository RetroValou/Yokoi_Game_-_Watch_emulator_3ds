
resolution_up = [400, 240]
resolution_down = [320, 240]
demi_resolution_up = [200, 240]

games_path = {
              "ball" :
                    { "ref" : "ac-01"
                    , "display_name" : "Ball"
                    , "Rom" : r'.\rom\decompress\gnw_ball\ac-01'
                    , "Visual" : [r'.\rom\decompress\gnw_ball\gnw_ball.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_ball\Background2NS.png']
                    , "transform_visual" : [[[2242, 121, 121], [1449, 69, 88]]]
                    , "date" : "1980-04-28"
                    , "console" : r'.\rom\decompress\gnw_ball\gnw_ball.png'
                    , "background_in_front" : True
                }
              
            , "Flagman" :
                    { "ref" : "fl-02"
                    , "display_name" : "Flagman"
                    , "Rom" : r'.\rom\decompress\gnw_flagman\fl-02'
                    , "Visual" : [r'.\rom\decompress\gnw_flagman\gnw_flagman.svg'] # list of screen visual
                    #, "transform_visual" : [[[1266, 28, 18], [835, 8, 32]]]}
                    , "transform_visual" : [[[1266, 28, 18], [900, 8, 32]]]
                    , "date" : "1980-06-05"
                    , "console" : r'.\rom\decompress\gnw_flagman\gnw_flagman.png'
                }
                              
            , "Vermin" :
                    { "ref" : "mt-03"
                    , "display_name" : "Vermin"
                    , "Rom" : r'.\rom\decompress\gnw_vermin\mt-03'
                    , "Visual" : [r'.\rom\decompress\gnw_vermin\gnw_vermin.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_vermin\Background2NS.png']
                    , "transform_visual" : [[[2329, 145, 184], [1505, 88, 125]]]
                    , "date" : "1980-07-10"
                    , "console" : r'.\rom\decompress\gnw_vermin\gnw_vermin.png'
                    , "background_in_front" : True                    
                    }
              
            , "Fire" :
                    { "ref" : "rc-04"
                    , "display_name" : "Fire"
                    , "Rom" : r'.\rom\decompress\gnw_fires\rc-04'
                    , "Visual" : [r'.\rom\decompress\gnw_fires\gnw_fires.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_fires\Background2NS.png']
                    , "transform_visual" : [[[2300, 150, 150], [1510, 125, 93]]]
                    , "date" : "1980-07-31"
                    , "console" : r'.\rom\decompress\gnw_fires\gnw_fires.png'
                    , "background_in_front" : True
                    }
              
            , "Judge" :
                    { "ref" : "ip-05"
                    , "display_name" : "Judge"
                    , "Rom" : r'.\rom\decompress\gnw_judge\ip-05'
                    , "Visual" : [r'.\rom\decompress\gnw_judge\gnw_judge.svg'] # list of screen visual
                    , "transform_visual" : [[[2359, 176, 183], [1548, 90, 166]]]
                    , "date" : "1980-10-04"
                    , "console" : r'.\rom\decompress\gnw_judge\gnw_judge.png'
                    }
                      
            , "Manhole" :
                    { "ref" : "MH_06"
                    , "display_name" : "Manhole"
                    , "Rom" : r'.\rom\decompress\gnw_manholeg\mh-06'
                    , "Visual" : [r'.\rom\decompress\gnw_manholeg\gnw_manholeg.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_manholeg\BackgroundNSM.png']
                    , "transform_visual" : [[[2396, 200, 160], [1607, 140, 130]]]
                    , "date" : "1981-01-27"
                    , "console" : r'.\rom\decompress\gnw_manholeg\gnw_manholeg.png'
                    , "background_in_front" : True 
                    }
              
            , "Helmet" :
                    { "ref" : "cn-17"
                    , "display_name" : "Helmet"
                    , "Rom" : r'.\rom\decompress\gnw_helmet\cn-17'
                    , "Visual" : [r'.\rom\decompress\gnw_helmet\gnw_helmet.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_helmet\BackgroundNS.png']
                    , "transform_visual" : [[[1145, 86, 101], [747, 58, 66]]]
                    , "date" : "1981-02-21"
                    , "console" : r'.\rom\decompress\gnw_helmet\gnw_helmet.png'
                    , "background_in_front" : True 
                    }
              
            , "Lion" :
                    { "ref" : "LN_08"
                    , "display_name" : "Lion"
                    , "Rom" : r'.\rom\decompress\gnw_lion\ln-08'
                    , "Visual" : [r'.\rom\decompress\gnw_lion\gnw_lion.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_lion\BackgroundNS.png']
                    , "transform_visual" : [[[2361, 146, 179], [1627, 145, 145]]]
                    , "date" : "1981-04-27"
                    , "console" : r'.\rom\decompress\gnw_lion\gnw_lion.png'
                    , "background_in_front" : True 
                    }
              
            , "Parachute" :
                    { "ref" : "pr-21"
                    , "display_name" : "Parachute"
                    , "Rom" : r'.\rom\decompress\gnw_pchute\pr-21'
                    , "Visual" : [r'.\rom\decompress\gnw_pchute\gnw_pchute.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_pchute\BackgroundNS.png']
                    , "transform_visual" : [[[1026, 25, 38], [699, 44, 40]]]
                    , "date" : "1981-06-16"
                    , "console" : r'.\rom\decompress\gnw_pchute\gnw_pchute.png'
                    , "background_in_front" : True
                    }
              
            , "octopus" :
                    { "ref" : "OC_22"
                    , "display_name" : "Octopus"
                    , "Rom" : r'.\rom\decompress\gnw_octopus\oc-22'
                    , "Visual" : [r'.\rom\decompress\gnw_octopus\gnw_octopus.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_octopus\BackgroundNS.png']
                    , "transform_visual" : [[[1026, 25, 38], [699, 44, 40]]]
                    , "date" : "1981-07-16"
                    , "console" : r'.\rom\decompress\gnw_octopus\gnw_octopus.png'
                    , "background_in_front" : True
                    }
              
             , "Popeye" :
                    { "ref" : "PP-23"
                    , "display_name" : "Popeye"
                    , "Rom" : r'.\rom\decompress\gnw_popeye\pp-23'
                    , "Visual" : [r'.\rom\decompress\gnw_popeye\gnw_popeye.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_popeye\BackgroundNS.png']
                    , "transform_visual" : [[[1317, 46, 48], [904, 60, 62]]]
                    , "date" : "1981-08-05"
                    , "console" : r'.\rom\decompress\gnw_popeye\gnw_popeye.png'
                    , "background_in_front" : True
                    }
              
            , "Chef" :
                    { "ref" : "fp-24"
                    , "display_name" : "Chef"
                    , "Rom" : r'.\rom\decompress\gnw_chef\fp-24'
                    , "Visual" : [r'.\rom\decompress\gnw_chef\gnw_chef.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_chef\BackgroundNS.png']
                    , "transform_visual" : [[[1282, 17, 14], [831, 23, 9]]]
                    , "date" : "1981-09-08"
                    , "console" : r'.\rom\decompress\gnw_chef\gnw_chef.png'
                    , "background_in_front" : True 
                    }
              
             , "Mickey_Mouse" :
                    { "ref" : "MC_25"
                    , "display_name" : "Mickey Mouse"
                    , "Rom" : r'.\rom\decompress\gnw_mmouse\mc-25'
                    , "Visual" : [r'.\rom\decompress\gnw_mmouse\gnw_mmouse.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_mmouse\BackgroundNS.png']
                    , "transform_visual" : [[[1405, 70, 90], [906, 60, 50]]]
                    , "date" : "1981-10-09"
                    , "console" : r'.\rom\decompress\gnw_mmouse\gnw_mmouse.png'
                    , "background_in_front" : True
                    }
              
             , "Egg" :
                    { "ref" : "EG_26"
                    , "display_name" : "Egg"
                    , "Rom" : r'.\rom\decompress\gnw_egg\mc-25'
                    , "Visual" : [r'.\rom\decompress\gnw_egg\gnw_egg.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_egg\BackgroundNS.png']
                    , "transform_visual" : [[[1415, 75, 95], [901, 55, 50]]]
                    , "date" : "1981-10-09"
                    , "console" : r'.\rom\decompress\gnw_egg\gnw_egg.png'
                    , "background_in_front" : True
                    }
              
            , "Fire_wide_screen" :
                    { "ref" : "fr-27"
                    , "display_name" : "Fire"
                    , "Rom" : r'.\rom\decompress\gnw_fire\fr-27'
                    , "Visual" : [r'.\rom\decompress\gnw_fire\gnw_fire.svg'] # list of screen visual
                    , "Background" : [r'.\rom\decompress\gnw_fire\BackgroundNS.png']
                    , "transform_visual" : [[[1315, 26, 44], [875, 40, 39]]]
                    , "date" : "1981-12-04"
                    , "console" : r'.\rom\decompress\gnw_fire\gnw_fire.png'
                    , "background_in_front" : True
                    }
              
            , "Turtle_Bridge" :
                    { "ref" : "tl-28"
                    , "display_name" : "Turtle Bridge"
                    , "Rom" : r'.\rom\decompress\gnw_tbridge\tl-28'
                    , "Visual" : [r'.\rom\decompress\gnw_tbridge\gnw_tbridge.svg'] # list of screen visual
                    , "Background" : [r'.\rom\decompress\gnw_tbridge\BackgroundNS.png']
                    , "alpha_bright" : 1.2
                    , "fond_bright" : 1.3
                    , "transform_visual" : [[[1286, 22, 51], [875, 53, 20]]]
                    , "date" : "1982-02-01"
                    , "console" : r'.\rom\decompress\gnw_tbridge\gnw_tbridge.png'
                    }
              
            , "Fire_Attack" :
                    { "ref" : "id-29"
                    , "display_name" : "Fire Attack"
                    , "Rom" : r'.\rom\decompress\gnw_fireatk\id-29'
                    , "Visual" : [r'.\rom\decompress\gnw_fireatk\gnw_fireatk.svg'] # list of screen visual
                    , "Background" : [r'.\rom\decompress\gnw_fireatk\BackgroundNS.png']
                    , "alpha_bright" : 1.1
                    , "fond_bright" : 1.2
                    , "shadow" : False
                    , "transform_visual" : [[[1350, 62, 67], [885, 50, 54]]]
                    , "date" : "1982-03-26"
                    , "console" : r'.\rom\decompress\gnw_fireatk\gnw_fireatk.png'
                    }
              
            , "Snoopy_Tennis" :
                { "ref" : "sp-30"
                , "display_name" : "Snoopy Tennis"
                , "Rom" : r'.\rom\decompress\gnw_stennis\sp-30'
                , "Visual" : [r'.\rom\decompress\gnw_stennis\gnw_stennis.svg'] # list of screen visual
                , "Background" : [r'.\rom\decompress\gnw_stennis\BackgroundNS.png']
                , "transform_visual" : [[[1294, 42, 39], [883, 49, 63]]]
                , "date" : "1982-04-28"
                , "console" : r'.\rom\decompress\gnw_stennis\gnw_stennis.png'
                , "background_in_front" : True 
                }

            ,"Oil_Panic" :
                { "ref" : "op-51"
                , "display_name" : "Oil Panic"
                , "date" : "1982-05-28"
                , "Rom" : r'.\rom\decompress\gnw_opanic\op-51'
                , "Visual" : [r'.\rom\decompress\gnw_opanic\gnw_opanic_top.svg'
                                , r'.\rom\decompress\gnw_opanic\gnw_opanic_bottom.svg'] # list of screen visual
                , "Background" :[r'.\rom\decompress\gnw_opanic\Screen-TopNS.png'
                                , r'.\rom\decompress\gnw_opanic\Screen-BottomNS.png']
                , "size_visual" : [resolution_up, [330, 240]]
                , "alpha_bright" : 1.2
                , "fond_bright" : 1.3
                , "transform_visual" : [[[1349, 33, 22], [899, 24, 58]], [[1371, 46, 51], [878, -18, 79]]]
                , "console" : r'.\rom\decompress\gnw_opanic\gnw_opanic.png'
                }
              
            ,"Donkey_kong" :
                { "ref" : "dk-52"
                , "display_name" : "Donkey Kong"
                , "date" : "1982-06-03"
                , "Rom" : r'.\rom\decompress\gnw_dkong\dk-52'
                , "Visual" : [r'.\rom\decompress\gnw_dkong\gnw_dkong_top.svg'
                                , r'.\rom\decompress\gnw_dkong\gnw_dkong_bottom.svg'] # list of screen visual
                , "Background" :[r'.\rom\decompress\gnw_dkong\Screen-TopNS.png'
                                , r'.\rom\decompress\gnw_dkong\Screen-BottomNS.png']
                , "size_visual" : [[330, 240], [330, 240]]
                , "transform_visual" : [[[1339, 16, 27], [870, 18, 35]], [[1319, 8, 15], [854, 11, 26]]]
                , "console" : r'.\rom\decompress\gnw_dkong\gnw_dkong.png'
                }

            , "Donkey_Kong_JR" :
                { "ref" : "dj-101"
                , "display_name" : "Donkey Kong JR"
                , "Rom" : r'.\rom\decompress\gnw_dkjr\dj-101'
                , "Visual" : [r'.\rom\decompress\gnw_dkjr\gnw_dkjr.svg'] # list of screen visual
                , "Background" :[r'.\rom\decompress\gnw_dkjr\BackgroundNS.png']
                , "transform_visual" : [[[1216, 1, 6], [798, 5, -5]]]
                , "date" : "1982-10-26"
                , "console" : r'.\rom\decompress\gnw_dkjr\gnw_dkjr.png'
                , "background_in_front" : True
                }
              
            ,"Mickey_Donald" :
                { "ref" : "dm-53"
                , "display_name" : "Mickey Donald"
                , "date" : "1982-11-12"
                , "Rom" : r'.\rom\decompress\gnw_mickdon\dm-53_565'
                , "Visual" : [r'.\rom\decompress\gnw_mickdon\gnw_mickdon_top.svg'
                                , r'.\rom\decompress\gnw_mickdon\gnw_mickdon_bottom.svg'] # list of screen visual
                , "Background" :[r'.\rom\decompress\gnw_mickdon\Screen-TopNS.png'
                                , r'.\rom\decompress\gnw_mickdon\Screen-BottomNS.png']
                , "size_visual" : [[350, 240], [350, 240]]
                , "transform_visual" : [[[1288, -8, 0], [844, 1, 26]], [[1273, -24, 1], [802, 13, -28]]]
                , "console" : r'.\rom\decompress\gnw_mickdon\gnw_mickdon.png'
                }
              
            ,"Green_House" :
                    { "ref" : "GH_54"
                    , "display_name" : "Green House"
                    , "date" : "1982-12-06"
                    , "Rom" : r'.\rom\decompress\gnw_ghouse\gh-54'
                    , "Visual" : [r'.\rom\decompress\gnw_ghouse\gnw_ghouse_top.svg'
                                    , r'.\rom\decompress\gnw_ghouse\gnw_ghouse_bottom.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_ghouse\Screen-TopNS.png'
                                    , r'.\rom\decompress\gnw_ghouse\Screen-BottomNS.png']
                    , "size_visual" : [[360, 240], [360, 240]]
                    , "transform_visual" : [[[1330, 16+6, 18+6], [886, 28, 41]], [[1329, 16, 17], [864, 14, 33]]]
                    , "console" : r'.\rom\decompress\gnw_ghouse\gnw_ghouse.png'
                    , "background_in_front" : True
                }

            ,"Donkey_Kong_2" :
                { "ref" : "jr-55"
                , "display_name" : "Donkey Kong 2"
                , "date" : "1983-03-07"
                , "Rom" : r'.\rom\decompress\gnw_dkong2\jr-55_560'
                , "Visual" : [r'.\rom\decompress\gnw_dkong2\gnw_dkong2_top.svg'
                                , r'.\rom\decompress\gnw_dkong2\gnw_dkong2_bottom.svg'] # list of screen visual
                , "Background" :[r'.\rom\decompress\gnw_dkong2\Screen-TopNS.png'
                                , r'.\rom\decompress\gnw_dkong2\Screen-BottomNS.png']
                , "size_visual" : [[344, 240], [344, 240]]
                , "transform_visual" : [[[1319, 0, 23], [827, 15, -5]], [[1314, 8, 10], [825, 3, 5]]]
                , "console" : r'.\rom\decompress\gnw_dkong2\gnw_dkong2.png'
                , "background_in_front" : True 
                }
              
            ,"Mario_Bros" :
                { "ref" : "mw-56"
                , "display_name" : "Mario Bros"
                , "date" : "1983-03-14"
                , "Rom" : r'.\rom\decompress\gnw_mario\mw-56'
                , "Visual" : [r'.\rom\decompress\gnw_mario\rework\gnw_mario_left.svg'
                                , r'.\rom\decompress\gnw_mario\rework\gnw_mario_right.svg'] # list of screen visual
                , "Background" :[r'.\rom\decompress\gnw_mario\rework\Screen-LeftNS.png'
                                , r'.\rom\decompress\gnw_mario\rework\Screen-RightNS.png']
                , "size_visual" : [[234, 240], [234, 240]]
                , "2_in_one_screen" : True
                , "shadow" : False
                , "transform_visual" : [[[1248, 17, 17], [782, 6, 0]], [[1217, 1, 2], [785, 3+10, 6-10]]]
                , "console" : r'.\rom\decompress\gnw_mario\Backdrop_2.png'
                , "background_in_front" : True
                }
              
            , "Mario_Cement_Factory_panorama" :
                    { "ref" : "cm-72"
                    , "display_name" : "Mario Cement Factory"
                    , "date" : "1983-04-28"
                    , "Rom" : r'.\rom\decompress\gnw_mariocmt\cm-72.program'
                    , "Visual" : [r'.\rom\decompress\gnw_mariocmt\gnw_mariocmt.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_mariocmt\cm-72.melody'
                    , "Background" : [r'.\rom\decompress\gnw_mariocmt\background2.png']
                    , "mask" : True
                    , "transform_visual" : [[[2074, -74, -80], [1240, -56, -80]]]
                    , "console" : r'.\rom\decompress\gnw_mariocmt\gnw_mariocmt.png'
                }
              
            , "Mario_Cement_Factory" :
                { "ref" : "ml-102"
                , "display_name" : "Mario Cement Factory"
                , "Rom" : r'.\rom\decompress\gnw_mariocm\ml-102_577'
                , "Visual" : [r'.\rom\decompress\gnw_mariocm\gnw_mariocm.svg'] # list of screen visual
                , "Background" : [r'.\rom\decompress\gnw_mariocm\BackgroundNS.png']
                , "transform_visual" : [[[1227, 8, 3], [816, 25, -7]]]
                , "date" : "1983-06-16"
                , "console" : r'.\rom\decompress\gnw_mariocm\gnw_mariocm.png'
                , "background_in_front" : True
                }

            ,"Rain_Shower" :
                { "ref" : "lp-57"
                , "display_name" : "Rain Shower"
                , "date" : "1983-08-10"
                , "Rom" : r'.\rom\decompress\gnw_rshower\lp-57'
                , "Visual" : [r'.\rom\decompress\gnw_rshower\gnw_rshower_left.svg'
                                , r'.\rom\decompress\gnw_rshower\gnw_rshower_right.svg'] # list of screen visual
                , "Background" :[r'.\rom\decompress\gnw_rshower\Screen-LeftNS.png'
                                , r'.\rom\decompress\gnw_rshower\Screen-RightNS.png']
                , "size_visual" : [[200, 240], [200, 240]]
                , "2_in_one_screen" : True
                , "shadow" : False
                , "alpha_bright" : 1.3
                , "fond_bright" : 1.3
                , "transform_visual" : [[[1280, 19, 47], [855, 47, 19]], [[1300, 48, 38], [853, 65, 12]]]
                , "console" : r'.\rom\decompress\gnw_rshower\gnw_rshower.png'
                }
              
            , "Manhole_wide_screen" :
                { "ref" : "NH_103"
                , "display_name" : "Manhole"
                , "Rom" : r'.\rom\decompress\gnw_manhole\nh-103'
                , "Visual" : [r'.\rom\decompress\gnw_manhole\gnw_manhole.svg'] # list of screen visual
                , "Background" : [r'.\rom\decompress\gnw_manhole\BackgroundNS.png']
                , "transform_visual" : [[[1290, 37, 36], [900, 56, 45]]]
                , "date" : "1983-08-24"
                , "console" : r'.\rom\decompress\gnw_manhole\gnw_manhole.png'
                , "background_in_front" : True 
                }
              
            , "Snoopy_Table_Top" :
                { "ref" : "sm-91"
                , "display_name" : "Snoopy"
                , "date" : "1983-08-30"
                , "Rom" : r'.\rom\decompress\gnw_snoopyp\sm-91.program'
                , "Visual" : [r'.\rom\decompress\gnw_snoopyp\gnw_snoopyp.svg'] # list of screen visual
                , "Melody_Rom" : r'.\rom\decompress\gnw_snoopyp\sm-91.melody'
                , "Background" : [r'.\rom\decompress\gnw_snoopyp\Background.png']
                , "mask" : True
                , "transform_visual" : [[[1328, 0, 0], [703, -9, -9]]]
                , "console" : r'.\rom\decompress\gnw_snoopyp\gnw_snoopyp.png'
                }

            , "Popeye_Table_Top" :
                    { "ref" : "PG_92"
                    , "display_name" : "Popeye"
                    , "date" : "1983-08-30"
                    , "Rom" : r'.\rom\decompress\gnw_popeyep\pg-92.program'
                    , "Visual" : [r'.\rom\decompress\gnw_popeyep\gnw_popeyep.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_popeyep\pg-92.melody'
                    , "Background" : [r'.\rom\decompress\gnw_popeyep\Background.png']
                    , "mask" : True
                    , "transform_visual" : [[[1328, 0, 0], [721, 0, 0]]]
                    , "console" : r'.\rom\decompress\gnw_popeyep\gnw_popeyep.png'
            }
              
            , "Donkey_Kong_Circus" :
                    { "ref" : "dc-95"
                    , "display_name" : "Donkey Kong Circus"
                    , "date" : "1983-09-06"
                    , "Rom" : r'.\rom\decompress\gnw_mmousep\dc-95.program'
                    , "Visual" : [r'.\rom\decompress\gnw_dkcirc\gnw_dkcirc.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_mmousep\dc-95.melody'
                    , "Background" : [r'.\rom\decompress\gnw_dkcirc\Background.png']
                    , "mask" : True
                    , "transform_visual" : [[[1620, 42, 25], [880, 37, 34]]]
                    , "console" : r'.\rom\decompress\gnw_dkcirc\gnw_dkcirc.png'
                }
              
            , "DK_JR_panorama" :
                    { "ref" : "cj-93"
                    , "display_name" : "Donkey Kong JR"
                    , "date" : "1983-10-07"
                    , "Rom" : r'.\rom\decompress\gnw_dkjrp\cj-93.program'
                    , "Visual" : [r'.\rom\decompress\gnw_dkjrp\gnw_dkjrp.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_dkjrp\cj-93.melody'
                    , "Background" : [r'.\rom\decompress\gnw_dkjrp\Background.png']
                    , "mask" : True
                    , "transform_visual" : [[[1650, 0, 0], [886, 0, 0]]]
                    , "console" : r'.\rom\decompress\gnw_dkjrp\gnw_dkjrp.png'
                }
              
              
            ,"Life_Boat" :
                    { "ref" : "tc-58"
                    , "display_name" : "Life Boat"
                    , "date" : "1983-10-25"
                    , "Rom" : r'.\rom\decompress\gnw_lboat\tc-58'
                    , "Visual" : [r'.\rom\decompress\gnw_lboat\rework\gnw_lboat_left.svg'
                                    , r'.\rom\decompress\gnw_lboat\rework\gnw_lboat_right.svg'] # list of screen visual
                    , "Background" :[r'.\rom\decompress\gnw_lboat\rework\Screen-LeftNS.png'
                                    , r'.\rom\decompress\gnw_lboat\rework\Screen-RightNS.png']
                    , "size_visual" : [[234, 240], [234, 240]]
                    , "2_in_one_screen" : True
                    , "shadow" : False
                    , "alpha_bright" : 1.2
                    , "transform_visual" : [[[1244, 18, 12], [836, 39, 21]], [[1273, 32, 27], [882, 86, 20]]]
                    , "console" : r'.\rom\decompress\gnw_lboat\gnw_lboat.png'
                }
              
            , "Mario_Bombs_Away" :
                    { "ref" : "tb-94"
                    , "display_name" : "Mario Bombs Away"
                    , "date" : "1983-11-10"
                    , "Rom" : r'.\rom\decompress\gnw_mbaway\tb-94.program'
                    , "Visual" : [r'.\rom\decompress\gnw_mbaway\gnw_mbaway.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_mbaway\tb-94.melody'
                    , "Background" : [r'.\rom\decompress\gnw_mbaway\background.png']
                    , "mask" : True
                    , "transform_visual" : [[[1650, 0, 0], [886, 0, 0]]]
                    , "console" : r'.\rom\decompress\gnw_mbaway\gnw_mbaway.png'
                }
              
            ,"Pinball" :
                    { "ref" : "PB_59"
                    , "display_name" : "Pinball"
                    , "date" : "1983-12-02"
                    , "Rom" : r'.\rom\decompress\gnw_pinball\pb-59.program'
                    , "Visual" : [r'.\rom\decompress\gnw_pinball\gnw_pinball_top.svg'
                                    , r'.\rom\decompress\gnw_pinball\gnw_pinball_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_pinball\pb-59.melody'
                    , "Background" : [r'.\rom\decompress\gnw_pinball\Screen-TopNS.png'
                                    , r'.\rom\decompress\gnw_pinball\Screen-BottomNS.png']
                    , "size_visual" : [[326, 240], [326, 240]]
                    , "transform_visual" : [[[1425, 64, 65], [928, 31, 80]], [[1438, 71, 71], [955, 31, 108]]]
                    , "console" : r'.\rom\decompress\gnw_pinball\gnw_pinball.png'
                }
              
            , "Mickey_Mouse_panorama" :
                    { "ref" : "dc-95"
                    , "display_name" : "Mickey Mouse"
                    , "date" : "1984-02-XX"
                    , "Rom" : r'.\rom\decompress\gnw_mmousep\dc-95.program'
                    , "Visual" : [r'.\rom\decompress\gnw_mmousep\gnw_mmousep.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_mmousep\dc-95.melody'
                    , "Background" : [r'.\rom\decompress\gnw_mmousep\Backgroundnew.png']
                    , "mask" : True
                    , "transform_visual" : [[[1701, 34, 41], [984, 50, 22]]]
                    , "console" : r'.\rom\decompress\gnw_mmousep\gnw_mmousep.png'
                }
              
            , "crab_grab" :
                    { "ref" : "ud-202"
                    , "display_name" : "Crab Grab"
                    , "date" : "1984-02-07"
                    , "Rom" : r'.\rom\decompress\gnw_cgrab\ud-202'
                    , "Visual" : [r'.\rom\decompress\gnw_cgrab\rework\gnw_cgrab_up.svg'
                                  , r'.\rom\decompress\gnw_cgrab\rework\gnw_cgrab_down.svg'] # up = 48% / down = 52%
                    , "Background" :[r'.\rom\decompress\gnw_cgrab\rework\background_up.png'
                                     , r'.\rom\decompress\gnw_cgrab\rework\background_down.png']
                    , "size_visual" : [resolution_down, resolution_down]
                    , "color_segment" : True
                    , "shadow" : False
                    , "alpha_bright" : 1
                    , "fond_bright" : 1.3
                    , "transform_visual" : [[[1003, 31, 31], [819-3, -26+3, 0]], [[1003, 31, 31], [906-20-22-4, 4, 22]]]
                    , "console" : r'.\rom\decompress\gnw_cgrab\gnw_cgrab.png'
            }
              
            , "Spitball_Sparky" :
                    { "ref" : "bu-201"
                    , "display_name" : "Spitball Sparky"
                    , "date" : "1984-02-21"
                    , "Rom" : r'.\rom\decompress\gnw_ssparky\bu-201'
                    , "Visual" : [r'.\rom\decompress\gnw_ssparky\rework\gnw_ssparky_up.svg'
                                    , r'.\rom\decompress\gnw_ssparky\rework\gnw_ssparky_down.svg'] # up = 48% / down = 52%
                    , "Background" :[r'.\rom\decompress\gnw_ssparky\rework\background_up.png'
                                    , r'.\rom\decompress\gnw_ssparky\rework\background_down.png']
                    , "size_visual" : [resolution_down, resolution_down]
                    , "color_segment" : True
                    , "shadow" : False
                    , "alpha_bright" : 1
                    , "fond_bright" : 1.3
                    , "transform_visual" : [[[1003, -32, -34], [864-10, -16+10, 0]], [[1003, -32, -34], [864-10, 0, -17+10]]]
                    , "console" : r'.\rom\decompress\gnw_ssparky\gnw_ssparky.png'
            }
              
             , "boxing" :
                    { "ref" : "BX_301"
                    , "display_name" : "Boxing"
                    , "date" : "1984-07-31"
                    , "Rom" : r'.\rom\decompress\gnw_boxing\bx-301_744.program'
                    , "Visual" : [r'.\rom\decompress\gnw_boxing\gnw_boxing.svg']
                    , "Melody_Rom" : r'.\rom\decompress\gnw_boxing\bx-301_744.melody'
                    , "Background" :[r'.\rom\decompress\gnw_boxing\BackgroundNS.png']
                    , "transform_visual" : [[[2291, -33, -23], [663, -36, -31]]]
                    , "console" : r'.\rom\decompress\gnw_boxing\gnw_boxing.png'
            }
              
            , "Donkey_Kong_3" :
                    { "ref" : "ak-302"
                    , "display_name" : "Donkey Kong 3"
                    , "date" : "1984-08-20"
                    , "Rom" : r'.\rom\decompress\gnw_dkong3\ak-302.program'
                    , "Visual" : [r'.\rom\decompress\gnw_dkong3\gnw_dkong3.svg']
                    , "Melody_Rom" : r'.\rom\decompress\gnw_dkong3\ak-302.melody'
                    , "Background" :[r'.\rom\decompress\gnw_dkong3\BackgroundNS.png']
                    , "transform_visual" : [[[2400, 36, 17], [724, -22, 16]]]
                    , "console" : r'.\rom\decompress\gnw_dkong3\gnw_dkong3.png'
                    , "background_in_front" : True 
            }
            
            , "Donkey_Kong_hockey" :
                    { "ref" : "hk-303"
                    , "display_name" : "Donkey Kong hockey"
                    , "date" : "1984-11-13"
                    , "Rom" : r'.\rom\decompress\gnw_dkhockey\hk-303.program'
                    , "Visual" : [r'.\rom\decompress\gnw_dkhockey\gnw_dkhockey.svg']
                    , "Melody_Rom" : r'.\rom\decompress\gnw_dkhockey\hk-303.melody'
                    , "Background" :[r'.\rom\decompress\gnw_dkhockey\BackgroundNS.png']
                    , "transform_visual" : [[[2460, 65, 48], [800, 40, 30]]]
                    , "console" : r'.\rom\decompress\gnw_dkhockey\gnw_dkhockey.png'
                    , "background_in_front" : True 
                }
                           
            ,"black_jack" :
                    { "ref" : "BJ-60"
                    , "display_name" : "Black Jack"
                    , "date" : "1985-02-15"
                    , "Rom" : r'.\rom\decompress\gnw_bjack\BJ-60.program'
                    , "Visual" : [r'.\rom\decompress\gnw_bjack\gnw_bjack_top.svg'
                                    , r'.\rom\decompress\gnw_bjack\gnw_bjack_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_bjack\BJ-60.melody'
                    , "Background" :[r'.\rom\decompress\gnw_bjack\Screen-TopNS.png'
                                    , r'.\rom\decompress\gnw_bjack\Screen-BottomNS.png']
                    , "transform_visual" : [[[1355, 33, 26], [898, 33, 48]], [[1346, 18, 32], [891, 4, 70]]]
                    , "size_visual" : [ resolution_up, [336, 240]]
                    , "console" : r'.\rom\decompress\gnw_bjack\gnw_bjack.png'
                }
              
            , "Tropical_Fish" :
                    { "ref" : "tf-104"
                    , "display_name" : "Tropical Fish"
                    , "Rom" : r'.\rom\decompress\gnw_tfish\tf-104'
                    , "Visual" : [r'.\rom\decompress\gnw_tfish\gnw_tfish.svg'] # list of screen visual
                    , "Background" : [r'.\rom\decompress\gnw_tfish\BackgroundNS.png']
                    , "transform_visual" : [[[1260, 4, 35], [885, 44, 42]]]
                    , "date" : "1985-07-08"
                    , "console" : r'.\rom\decompress\gnw_tfish\gnw_tfish.png'
                }
              
            , "Squish" :
                    { "ref" : "mg-61"
                    , "display_name" : "Squish"
                    , "date" : "1986-04-XX"
                    , "Rom" : r'.\rom\decompress\gnw_squish\mg-61'
                    , "Visual" : [r'.\rom\decompress\gnw_squish\gnw_squish_top.svg'
                                    , r'.\rom\decompress\gnw_squish\gnw_squish_bottom.svg'] # list of screen visual
                    , "Background" : [r'.\rom\decompress\gnw_squish\Screen-TopNS.png'
                                    , r'.\rom\decompress\gnw_squish\Screen-BottomNS.png']
                    , "transform_visual" : [[[1373, 42, 35], [896, 29, 50]], [[1357, 22, 39], [896, 20, 59]]]
                    , "size_visual" : [resolution_up, [360, 240]]
                    , "console" : r'.\rom\decompress\gnw_squish\gnw_squish.png'
                    , "background_in_front" : True
                }

            , "Super_Mario_Bros_cristal_screen" :
                    { "ref" : "ym-801"
                    , "display_name" : "Super Mario Bros (Cristal Screen)"
                    , "Rom" : r'.\rom\decompress\gnw_smb\ym-801.program'
                    , "Melody_Rom" : r'.\rom\decompress\gnw_smb\ym-801.melody'
                    , "Visual" : [r'.\rom\decompress\gnw_smb\gnw_smb.svg'] # list of screen visual
                    , "Background" : [r'.\rom\decompress\gnw_smb\BackgroundNS.png']
                    , "size_visual" : [[400, 260]]
                    , "transform_visual" : [[[1266, 28, 18], [835, 8, 32]]]
                    , "shadow" : False
                    , "alpha_bright" : 1.2
                    , "date" : "1986-06-25"
                    , "console" : r'.\rom\decompress\gnw_smb\gnw_smb.png'
                    , "camera" : True
                }
                            
            , "Ice_Climber_cristal_screen" :
                    { "ref" : "dr-802"
                    , "display_name" : "Climber (Cristal Screen)"
                    , "date" : "1986-11-19"
                    , "Rom" : r'.\rom\decompress\gnw_climber\dr-802.program'
                    , "Visual" : [r'.\rom\decompress\gnw_climber\gnw_climber.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_climber\dr-802.melody'
                    , "Background" : [r'.\rom\decompress\gnw_climber\ClimberCCNS.png']
                    , "transform_visual" : [[[1321, 52, 44], [861, 27, 30]]]             
                    , "size_visual" : [[400, 250]]
                    , "console" : r'.\rom\decompress\gnw_climber\gnw_climber.png'
                    , "camera" : True
                }

            ,"Balloon_Fight_cristal_screen" :
                    { "ref" : "BF_107"
                    , "display_name" : "Balloon Fight (Cristal Screen)"
                    , "date" : "1986-11-19"
                    , "Rom" : r'.\rom\decompress\gnw_bfight\bf-803.program'
                    , "Visual" : [r'.\rom\decompress\gnw_bfight\gnw_bfight.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_bfight\bf-803.melody'
                    , "Background" : [r'.\rom\decompress\gnw_bfight\BackgroundNS.png']
                    , "transform_visual" : [[[1281, 25, 32], [917, 56, 60]]]
                    , "console" : r'.\rom\decompress\gnw_bfight\gnw_bfight.png'
                    , "camera" : True
                }
              
            , "bomb_sweeper" :
                    { "ref" : "bd-62"
                    , "display_name" : "Bomb Sweeper"
                    , "date" : "1987-06-XX"
                    , "Rom" : r'.\rom\decompress\gnw_bsweep\bd-62.program'
                    , "Visual" : [r'.\rom\decompress\gnw_bsweep\rework\gnw_bsweep_top.svg'
                                    , r'.\rom\decompress\gnw_bsweep\rework\gnw_bsweep_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_bsweep\bd-62.melody'
                    , "Background" :[r'.\rom\decompress\gnw_bsweep\Screen-TopNS.png'
                                    , r'.\rom\decompress\gnw_bsweep\Screen-BottomNS.png']
                    , "transform_visual" : [[[1490, 116, 78], [986, 78, 91]], [[1478, 102, 80], [937, 37, 83]]]
                    , "size_visual" : [ resolution_up, [344, 240]]
                    , "console" : r'.\rom\decompress\gnw_bsweep\gnw_bsweep.png'
                }
              
            ,"Safe_Buster" :
                    { "ref" : "jb-63"
                    , "display_name" : "Safe Buster"
                    , "date" : "1988-01-XX"
                    , "Rom" : r'.\rom\decompress\gnw_sbuster\jb-63.program'
                    , "Visual" : [r'.\rom\decompress\gnw_sbuster\gnw_sbuster_top.svg'
                                    , r'.\rom\decompress\gnw_sbuster\gnw_sbuster_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_sbuster\jb-63.melody'
                    , "Background" :[r'.\rom\decompress\gnw_sbuster\Screen-TopNS.png'
                                    , r'.\rom\decompress\gnw_sbuster\Screen-BottomNS.png']
                    , "size_visual" : [ [330, 240], [340, 240]]
                    , "transform_visual" : [[[1463, 84, 83], [938, 31, 90]], [[1448, 81, 71], [957, 34, 106]]]
                    , "console" : r'.\rom\decompress\gnw_sbuster\gnw_sbuster.png'
                }
              
              
            , "Super_Mario_Bros" :
                    { "ref" : "ym-801"
                    , "display_name" : "Super Mario Bros"
                    , "Rom" : r'.\rom\decompress\gnw_smbn\ym-801.program'
                    , "Melody_Rom" : r'.\rom\decompress\gnw_smbn\ym-801.melody'
                    , "Visual" : [r'.\rom\decompress\gnw_smbn\rework\gnw_smbn.svg'] # list of screen visual
                    , "Background" : [r'.\rom\decompress\gnw_smbn\rework\BackgroundNS.png']
                    , "size_visual" : [[400, 260]]
                    , "transform_visual" : [[[1266, 28, 18], [835, 8, 32]]]
                    , "shadow" : False
                    , "alpha_bright" : 1.2
                    , "date" : "1988-03-XX"
                    , "console" : r'.\rom\decompress\gnw_smbn\gnw_smbn.png'
                }

            , "Ice_Climber" :
                    { "ref" : "dr-802"
                    , "display_name" : "Climber"
                    , "date" : "1988-03-XX"
                    , "Rom" : r'.\rom\decompress\gnw_climber\dr-802.program'
                    , "Visual" : [r'.\rom\decompress\gnw_climbern\gnw_climbern.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_climber\dr-802.melody'
                    , "Background" : [r'.\rom\decompress\gnw_climbern\BackgroundNS.png']
                    , "transform_visual" : [[[1321, 52, 44], [861, 27, 30]]]             
                    , "size_visual" : [[400, 250]]
                    , "console" : r'.\rom\decompress\gnw_climbern\gnw_climbern.png'
                }    

            ,"Balloon_Fight" :
                    { "ref" : "BF_107"
                    , "display_name" : "Balloon Fight"
                    , "date" : "1988-03-XX"
                    , "Rom" : r'.\rom\decompress\gnw_bfightn\bf-803.program'
                    , "Visual" : [r'.\rom\decompress\gnw_bfightn\gnw_bfightn.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_bfightn\bf-803.melody'
                    , "Background" : [r'.\rom\decompress\gnw_bfightn\BackgroundNS.png']
                    , "transform_visual" : [[[1281, 25, 32], [917, 56, 60]]]
                    , "console" : r'.\rom\decompress\gnw_bfightn\gnw_bfightn.png'
                }
              
            ,"Zelda" :
                    { "ref" : "zl-65"
                    , "display_name" : "Zelda"
                    , "date" : "1988-08-XX"
                    , "Rom" : r'.\rom\decompress\gnw_zelda\zl-65.program'
                    , "Visual" : [r'.\rom\decompress\gnw_zelda\rework\gnw_zelda_top.svg'
                                    , r'.\rom\decompress\gnw_zelda\rework\gnw_zelda_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_zelda\zl-65.melody'
                    , "Background" : [r'.\rom\decompress\gnw_zelda\rework\Screen-TopNS.png'
                                    , r'.\rom\decompress\gnw_zelda\rework\Screen-BottomNS.png']
                    , "size_visual" : [resolution_up, [340, 240]]
                    , "transform_visual" : [[[1393, 38, 44], [933, 36, 80]], [[1393, 46, 51], [915, 36, 62]]]
                    , "shadow" : True
                    , "console" : r'.\rom\decompress\gnw_zelda\gnw_zelda.png'
                }
              
            ,"Gold_Cliff" :
                    { "ref" : "mv-64"
                    , "display_name" : "Gold Cliff"
                    , "date" : "1988-10-XX"
                    , "Rom" : r'.\rom\decompress\gnw_gcliff\mv-64.program'
                    , "Visual" : [r'.\rom\decompress\gnw_gcliff\gnw_gcliff_top.svg'
                                    , r'.\rom\decompress\gnw_gcliff\gnw_gcliff_bottom.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_gcliff\mv-64.melody'
                    , "Background" :[r'.\rom\decompress\gnw_gcliff\Screen-TopNS.png'
                                    , r'.\rom\decompress\gnw_gcliff\Screen-BottomNS.png']
                    , "transform_visual" : [[[1360, 31, 33], [890, 33, 40]], [[1348, 32, 20], [870, 10, 43]]]
                    , "size_visual" : [ [340, 240], [340, 240]]
                    , "console" : r'.\rom\decompress\gnw_gcliff\gnw_gcliff.png'
                }
              
            , "Mario_the_Juggle" :
                    { "ref" : "mb-108"
                    , "display_name" : "Mario the Juggler"
                    , "date" : "1991-10-XX"
                    , "Rom" : r'.\rom\decompress\gnw_mariotj\mb-108.program'
                    , "Visual" : [r'.\rom\decompress\gnw_mariotj\gnw_mariotj.svg'] # list of screen visual
                    , "Melody_Rom" : r'.\rom\decompress\gnw_mariotj\mb-108.melody'
                    , "Background" : [r'.\rom\decompress\gnw_mariotj\BackgroundNSm.png']
                    , "transform_visual" : [[[1410, 67, 53], [960, 54, 52]]]
                    , "console" : r'.\rom\decompress\gnw_mariotj\gnw_mariotj.png'
                }
              
              
}