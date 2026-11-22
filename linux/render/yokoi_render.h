#pragma once

#include <array>
#include <vector>
#include "render/yokoi_graphics.h"
#include "core/yokoi_ui.h"
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
            : renderer_(renderer), panel_(panel), font_(renderer) {}
        void load(const GW_rom &game);
        void draw(const GW_rom &game, const std::vector<Segment> &segments, bool menu);
        void draw(const GW_rom &game, const std::vector<Segment> &segments, const UiState &ui,
                  bool settings_overlay = false);
        void set_panel(Panel panel) { panel_ = panel; }

    private:
        struct PanelLayout
        {
            SDL_FRect bounds{0, 0, logical_width, logical_height};
            SDL_FPoint offset[2]{};
            SDL_FPoint size[2]{};
            float divisor = 1.f;
            float zoom = 1.f;
            int lcd_count = 0;
            int selected_screen = -1;
            bool match_lcd = false;
        };
        struct OutputTransform
        {
            float scale = 1.f;
            SDL_FPoint offset{};
            bool snap = false;
            SDL_FRect rect(const SDL_FRect &area) const;
        };
        PanelLayout layout_panel(const GW_rom &game, const std::vector<Segment> &segments,
                                 const UiState &ui, Panel panel) const;
        void draw_panel(const GW_rom &game, const std::vector<Segment> &segments, const UiState &ui,
                        Panel panel, const PanelLayout &layout, const OutputTransform &output);
        void draw_menu(const GW_rom &game, const UiState &ui, const OutputTransform &output);
        void draw_settings(const UiState &ui, bool dim_background);
        void draw_game(const GW_rom &game, const std::vector<Segment> &segments,
                       const PanelLayout &layout, const OutputTransform &output);

        SDL_Renderer *renderer_;
        Panel panel_;
        Font font_;
        Texture atlas_{nullptr, SDL_DestroyTexture};
        Texture background_{nullptr, SDL_DestroyTexture};
        Texture console_{nullptr, SDL_DestroyTexture};
        Texture marking_{nullptr, SDL_DestroyTexture};
        std::array<Texture, 5> colors_{{{nullptr, SDL_DestroyTexture},
                                       {nullptr, SDL_DestroyTexture},
                                       {nullptr, SDL_DestroyTexture},
                                       {nullptr, SDL_DestroyTexture},
                                       {nullptr, SDL_DestroyTexture}}};
    };
}
