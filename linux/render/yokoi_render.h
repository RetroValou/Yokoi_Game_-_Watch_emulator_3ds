#pragma once

#include <array>
#include <vector>
#include "render/yokoi_graphics.h"
#include "std/load_file.h"

namespace yokoi
{
    inline constexpr int logical_width = 640;
    inline constexpr int logical_height = 480;

    enum class Panel { combined, top, bottom };

    // Owns a title's textures; the SDL renderer and ROM metadata outlive it.
    class GameRenderer
    {
    public:
        explicit GameRenderer(SDL_Renderer *renderer, Panel panel = Panel::combined)
            : renderer_(renderer), panel_(panel) {}
        void load(const GW_rom &game);
        void draw(const GW_rom &game, const std::vector<Segment> &segments, bool menu);

    private:
        void draw_menu(const GW_rom &game);
        void draw_console(const GW_rom &game, bool in_menu = false);
        void draw_game(const GW_rom &game, const std::vector<Segment> &segments);

        SDL_Renderer *renderer_;
        Panel panel_;
        Texture atlas_{nullptr, SDL_DestroyTexture};
        Texture background_{nullptr, SDL_DestroyTexture};
        Texture console_{nullptr, SDL_DestroyTexture};
        std::array<Texture, 5> colors_{{{nullptr, SDL_DestroyTexture},
                                       {nullptr, SDL_DestroyTexture},
                                       {nullptr, SDL_DestroyTexture},
                                       {nullptr, SDL_DestroyTexture},
                                       {nullptr, SDL_DestroyTexture}}};
    };
}
