#include "render/yokoi_render.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include "core/yokoi_sdl.h"
#include "core/yokoi_version.h"
#include "std/GW_info_reader.h"
#include "std/settings.h"

namespace yokoi
{
    SDL_FRect GameRenderer::OutputTransform::rect(const SDL_FRect &area) const
    {
        if (!snap)
            return SDL_FRect{offset.x + area.x * scale, offset.y + area.y * scale,
                             area.w * scale, area.h * scale};
        // Round edges rather than widths so adjacent layers agree on their pixel boundaries.
        const float left = std::round(offset.x + area.x * scale);
        const float top = std::round(offset.y + area.y * scale);
        const float right = std::round(offset.x + (area.x + area.w) * scale);
        const float bottom = std::round(offset.y + (area.y + area.h) * scale);
        return SDL_FRect{left, top, right - left, bottom - top};
    }

    static std::optional<int> shared_lcd_reference(const GW_rom &game, const std::vector<Segment> &segments)
    {
        const auto *info = game.segment_info;
        if ((info[3] & 2) || !std::any_of(segments.begin(), segments.end(),
                                        [](const Segment &segment) { return segment.screen == 1; }))
            return std::nullopt;
        const uint64_t top = uint64_t(info[4]) * info[7];
        const uint64_t bottom = uint64_t(info[6]) * info[5];
        if (!top || !bottom)
            throw std::runtime_error("Invalid dual-LCD dimensions");
        // Correct small extraction differences, not intentionally different LCD shapes.
        if (std::max(top, bottom) * 100 > std::min(top, bottom) * 103)
            return std::nullopt;
        return top <= bottom ? 0 : 1;
    }

    void GameRenderer::load(const GW_rom &game)
    {
        const bool mask_atlas = (game.segment_info[3] & 1) != 0;
        atlas_ = load_texture(renderer_, game.path_segment, mask_atlas ? -1 : 0xffffff);
        marking_.reset();
        if (!mask_atlas)
            marking_ = load_texture(renderer_, game.path_segment, 0x101010);
        background_.reset();
        console_.reset();
        if (!game.path_background.empty())
            background_ = load_texture(renderer_, game.path_background);
        if (!game.path_console.empty())
            console_ = load_texture(renderer_, game.path_console);
        constexpr int palette[] = {0x080908, 0x9992e7, 0x58b9a0, 0xff677c, 0x3db8e4};
        for (auto &color : colors_)
            color.reset();
        // Mask atlases carry their own colors; monochrome atlases need tinted copies.
        if (!mask_atlas)
        {
            for (size_t i = 0; i < game.size_segment; ++i)
            {
                const auto color = game.segment[i].color_index;
                if (color > 0 && color < 5 && !colors_[color])
                    colors_[color] = load_texture(renderer_, game.path_segment, palette[color]);
            }
        }
    }

    void GameRenderer::draw(const GW_rom &game, const std::vector<Segment> &segments, bool menu)
    {
        UiState ui;
        ui.mode = menu ? AppMode::select : AppMode::game;
        draw(game, segments, ui);
    }

    void GameRenderer::draw(const GW_rom &game, const std::vector<Segment> &segments,
                            const UiState &ui, bool settings_overlay)
    {
        const auto requested_panel = panel_;
        bool background_dimmed = false;
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        if (requested_panel == Panel::combined)
        {
            int output_width, output_height, previous_width, previous_height;
            require_sdl(SDL_GetRendererOutputSize(renderer_, &output_width, &output_height) == 0,
                        "Get panel output size");
            SDL_RenderGetLogicalSize(renderer_, &previous_width, &previous_height);
            require_sdl(SDL_RenderSetLogicalSize(renderer_, 0, 0) == 0, "Use full panel output");
            const auto fit_panel = [&](const SDL_FRect &source, int y, int available_height)
            {
                const float scale = std::min(output_width / source.w, available_height / source.h);
                const float width = source.w * scale, height = source.h * scale;
                return SDL_FRect{(output_width - width) / 2, y + (available_height - height) / 2, width, height};
            };
            const auto draw_fitted = [&](Panel panel, const PanelLayout &layout, const SDL_FRect &area)
            {
                if (area.w <= 0 || area.h <= 0)
                    return;
                const auto &source = layout.bounds;
                const float scale = area.w / source.w;
                const OutputTransform output{scale, {area.x - source.x * scale, area.y - source.y * scale}, true};
                const auto bounds = output.rect(source);
                const SDL_Rect clip{int(bounds.x), int(bounds.y), int(bounds.w), int(bounds.h)};
                require_sdl(SDL_RenderSetClipRect(renderer_, &clip) == 0, "Clip fitted panel");
                draw_panel(game, segments, ui, panel, layout, output);
                require_sdl(SDL_RenderSetClipRect(renderer_, nullptr) == 0, "Restore full output clip");
            };
            const auto top_layout = layout_panel(game, segments, ui, Panel::top);
            const int top_height = output_height / 2;
            auto top = fit_panel(top_layout.bounds, 0, ui.top_only ? output_height : top_height);
            PanelLayout bottom_layout;
            SDL_FRect bottom{};
            if (!ui.top_only)
            {
                bottom_layout = layout_panel(game, segments, ui, Panel::bottom);
                bottom = fit_panel(bottom_layout.bounds, top_height, output_height - top_height);
                if (top_layout.match_lcd && top.h > 0)
                {
                    // Start each LCD on an integer row so equal-height LCDs round identically.
                    const float rows = std::round(top.h);
                    top.y = std::floor((output_height - 2 * rows) / 2);
                    bottom = top;
                    bottom.y += rows;
                }
            }
            draw_fitted(Panel::top, top_layout, top);
            if (!ui.top_only)
                draw_fitted(Panel::bottom, bottom_layout, bottom);
            if (settings_overlay)
            {
                SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 170);
                SDL_RenderFillRect(renderer_, nullptr);
                SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
                background_dimmed = true;
            }
            // Restore the UI viewport, which also keeps SDL's pointer coordinates consistent.
            require_sdl(SDL_RenderSetLogicalSize(renderer_, previous_width, previous_height) == 0,
                        "Restore UI logical size");
        }
        else
        {
            OutputTransform output;
            int previous_width = 0, previous_height = 0;
            if (ui.mode == AppMode::game || requested_panel == Panel::bottom)
            {
                int output_width, output_height;
                require_sdl(SDL_GetRendererOutputSize(renderer_, &output_width, &output_height) == 0,
                            "Get artwork output size");
                if (output_width == 1024 && output_height == 768)
                {
                    output = {float(output_width) / logical_width, {}, true};
                    SDL_RenderGetLogicalSize(renderer_, &previous_width, &previous_height);
                    require_sdl(SDL_RenderSetLogicalSize(renderer_, 0, 0) == 0, "Use native artwork coordinates");
                }
            }
            const auto layout = layout_panel(game, segments, ui, requested_panel);
            draw_panel(game, segments, ui, requested_panel, layout, output);
            // Keep the settings overlay and SDL input mapping in logical coordinates.
            if (output.snap)
                require_sdl(SDL_RenderSetLogicalSize(renderer_, previous_width, previous_height) == 0,
                            "Restore UI logical size");
        }
        if (settings_overlay)
            draw_settings(ui, !background_dimmed);
        SDL_RenderPresent(renderer_);
    }

    void GameRenderer::draw_panel(const GW_rom &game, const std::vector<Segment> &segments, const UiState &ui,
                                  Panel panel, const PanelLayout &layout, const OutputTransform &output)
    {
        if (layout.lcd_count)
            draw_game(game, segments, layout, output);
        else if (panel == Panel::bottom)
        {
            if (console_ && game.console_info)
            {
                const auto *info = game.console_info;
                copy_texture(renderer_, console_.get(), SDL_Rect{info[2], info[3], info[4], info[5]},
                             output.rect(layout.bounds));
            }
        }
        else
            draw_menu(game, ui, output);
    }

    void GameRenderer::draw_menu(const GW_rom &game, const UiState &ui, const OutputTransform &output)
    {
        const auto draw_text = [&](float x, float y, const std::string &text, float scale, uint32_t color)
        {
            font_.draw(output.offset.x + x * output.scale, output.offset.y + y * output.scale,
                       text, scale * output.scale, color);
        };
        const auto centered = [&](const std::string &text, float baseline, float scale, uint32_t color)
        {
            draw_text((logical_width - text_width(text) * scale) / 2, baseline - 10 * scale, text, scale, color);
        };
        constexpr const char *manufacturers[] = {"Nintendo", "Tronica", "Elektronika"};
        centered(manufacturers[game.manufacturer], 77, 2.1f, 0x888888);
        centered(game.name, 144, std::min(4.8f, 608.f / std::max(1.f, text_width(game.name))), 0xffffff);
        centered(game.date, 195, 2.5f, 0xcccccc);
        if (ui.mode == AppMode::select)
        {
            centered("Up/Down: manufacturer", 272, 2.5f, 0x888888);
            centered("Left/Right: game", 320, 2.5f, 0x888888);
            centered("Tap center or press A", 368, 2.5f, 0x888888);
        }
        else
        {
            centered("Start:", 256, 2.5f, 0xffffff);
            const auto left = ui.load_save ? "Load fresh" : "> Load fresh";
            const auto right = ui.has_save ?
                (ui.load_save ? "> Load savestate" : "Load savestate") : "No savestate";
            const float scale = std::min(3.f, 288.f / std::max(text_width(left), text_width(right)));
            draw_text(160 - text_width(left) * scale / 2, 298, left, scale, 0xcccccc);
            draw_text(480 - text_width(right) * scale / 2, 298, right, scale, 0xcccccc);
            centered("B: back", 371, 2.1f, 0x888888);
        }
    }

    void GameRenderer::draw_settings(const UiState &ui, bool dim_background)
    {
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        if (dim_background)
        {
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 170);
            SDL_RenderFillRect(renderer_, nullptr);
        }
        SDL_FRect box{56, 20, 528, 440};
        SDL_SetRenderDrawColor(renderer_, 38, 38, 38, 255);
        SDL_RenderFillRectF(renderer_, &box);
        font_.draw(80, 36, "Settings", 3);
        const auto items = settings_items(ui, panel_ != Panel::combined);
        int alpha_index = 1, background_index = 0;
        for (size_t i = 0; i < segment_alphas.size(); ++i)
            if (g_settings.segment_marking_alpha == segment_alphas[i]) alpha_index = int(i);
        for (int i = 0; i < NUM_BG_PRESETS; ++i)
            if (g_settings.background_color == BACKGROUND_PRESETS[i].color) background_index = i;
        for (size_t i = 0; i < items.size(); ++i)
        {
            std::string label, value;
            switch (items[i])
            {
            case Setting::segments: label = "Segments"; value = segment_names[alpha_index]; break;
            case Setting::background: label = "Background color"; value = BACKGROUND_PRESETS[background_index].name; break;
            case Setting::controller_layout:
                label = "Controller layout";
                value = ui.controller_layout == ControllerLayout::rgds ? "RGDS" : "Default";
                break;
            case Setting::swap: label = "Swap screens"; value = ui.swap_screens ? "On" : "Off"; break;
            case Setting::quit_game: label = "Quit Game"; break;
            case Setting::top_only: label = ui.top_only ? "Show both screens" : "Top screen only"; break;
            case Setting::exit_app: label = "Exit App"; break;
            case Setting::close: label = "Close"; break;
            }
            const float y = 78 + i * 50;
            if (i == ui.settings_row)
            {
                SDL_FRect selected{64, y, 512, 46};
                SDL_SetRenderDrawColor(renderer_, 65, 79, 102, 255);
                SDL_RenderFillRectF(renderer_, &selected);
            }
            font_.draw(80, y + 12, label, 2);
            const float scale = std::min(2.f,
                (460.f - text_width(label) * 2) / std::max(1.f, text_width(value)));
            font_.draw(560 - text_width(value) * scale, y + 12, value, scale, 0xcccccc);
        }
        font_.draw(80, 426, "Up/Down: select  Left/Right: change  A: choose  B/R: close", 1);
        const std::string version_label = std::string("Yokoi ") + app_version;
        font_.draw(560 - text_width(version_label), 444, version_label, 1, 0x888888);
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
    }

    GameRenderer::PanelLayout GameRenderer::layout_panel(const GW_rom &game, const std::vector<Segment> &segments,
                                                          const UiState &ui, Panel panel) const
    {
        PanelLayout layout;
        const auto fit_canvas = [&](float width, float height)
        {
            if (width <= 0 || height <= 0)
                throw std::runtime_error("Invalid panel artwork dimensions");
            layout.zoom = std::min(logical_width / std::max(1.f, width), logical_height / std::max(1.f, height));
            layout.bounds = {(logical_width - width * layout.zoom) / 2,
                             (logical_height - height * layout.zoom) / 2,
                             width * layout.zoom, height * layout.zoom};
        };
        const auto fit_console = [&]
        {
            if (console_ && game.console_info)
                fit_canvas(game.console_info[4], game.console_info[5]);
        };
        if (ui.mode != AppMode::game)
        {
            if (panel == Panel::bottom)
                fit_console();
            return layout;
        }

        const auto *info = game.segment_info;
        const float divisor = std::max(1, int(info[2]));
        const bool dual_lcd = std::any_of(segments.begin(), segments.end(),
                                         [](const Segment &segment) { return segment.screen == 1; });
        // Horizontal games keep both LCDs together; only clamshell games split.
        const bool horizontal = (info[3] & 2) != 0;
        const bool split = dual_lcd && !horizontal;
        if (panel == Panel::bottom && !split)
        {
            fit_console();
            return layout;
        }

        layout.divisor = divisor;
        layout.lcd_count = dual_lcd ? 2 : 1;
        layout.size[0] = {info[4] / divisor, info[5] / divisor};
        if (dual_lcd)
            layout.size[1] = {info[6] / divisor, info[7] / divisor};
        if (split)
        {
            layout.selected_screen = panel == Panel::bottom ? 1 : 0;
            const auto shared_screen = ui.top_only ? std::nullopt : shared_lcd_reference(game, segments);
            layout.match_lcd = shared_screen.has_value();
            const auto &size = layout.size[shared_screen.value_or(layout.selected_screen)];
            fit_canvas(size.x, size.y);
        }
        else if (dual_lcd)
        {
            const auto &left = layout.size[0], &right = layout.size[1];
            const float height = std::max(left.y, right.y);
            layout.offset[0] = {0, (height - left.y) / 2};
            layout.offset[1] = {left.x, (height - right.y) / 2};
            fit_canvas(left.x + right.x, height);
        }
        else
            fit_canvas(layout.size[0].x, layout.size[0].y);
        return layout;
    }

    void GameRenderer::draw_game(const GW_rom &game, const std::vector<Segment> &segments,
                                  const PanelLayout &layout, const OutputTransform &output)
    {
        const auto *info = game.segment_info;
        const auto &size = layout.size;
        const auto &offset = layout.offset;
        const float divisor = layout.divisor, zoom = layout.zoom;
        const int count = layout.lcd_count, selected_screen = layout.selected_screen;
        const auto want_screen = [&](int screen) { return selected_screen < 0 || screen == selected_screen; };
        const auto destination = [&](float x, float y, float w, float h)
        {
            if (layout.match_lcd)
            {
                return SDL_FRect{layout.bounds.x + x / size[selected_screen].x * layout.bounds.w,
                                 layout.bounds.y + y / size[selected_screen].y * layout.bounds.h,
                                 w / size[selected_screen].x * layout.bounds.w,
                                 h / size[selected_screen].y * layout.bounds.h};
            }
            return SDL_FRect{layout.bounds.x + x * zoom, layout.bounds.y + y * zoom,
                             w * zoom, h * zoom};
        };
        const auto *background_info = game.background_info;
        const auto copy_artwork = [&](SDL_Texture *texture, SDL_Rect source, const SDL_FRect &dest)
        {
            copy_texture(renderer_, texture, source, output.rect(dest));
        };
        const auto draw_background = [&](float shift = 0.f)
        {
            if (!background_ || !background_info)
                return;
            for (int screen = 0; screen < count; ++screen)
            {
                if (!want_screen(screen))
                    continue;
                const auto *background = background_info + bg_jump(screen);
                SDL_Rect source{background[0], background[1], background[2], background[3]};
                const auto dest = destination(offset[screen].x + shift, offset[screen].y + shift,
                                              size[screen].x, size[screen].y);
                copy_artwork(background_.get(), source, dest);
            }
        };
        for (int screen = 0; screen < count; ++screen)
        {
            if (!want_screen(screen))
                continue;
            const auto dest = output.rect(destination(offset[screen].x, offset[screen].y,
                                                       size[screen].x, size[screen].y));
            const uint32_t color = (info[3] & 1) ? 0x080908 : g_settings.background_color;
            SDL_SetRenderDrawColor(renderer_, (color >> 16) & 255, (color >> 8) & 255, color & 255, 255);
            SDL_RenderFillRectF(renderer_, &dest);
        }
        // Like Android, cast the artwork's shadow onto the LCD fill before compositing the other layers.
        if (background_ && background_info && background_info[i_bg_shadow(count)] == 1)
        {
            auto *texture = background_.get();
            require_sdl(SDL_SetTextureColorMod(texture, 0, 0, 0) == 0, "Background shadow color");
            require_sdl(SDL_SetTextureAlphaMod(texture, 0x24) == 0, "Background shadow opacity");
            draw_background(6.f);
            require_sdl(SDL_SetTextureColorMod(texture, 255, 255, 255) == 0, "Restore background color");
            require_sdl(SDL_SetTextureAlphaMod(texture, 255) == 0, "Restore background opacity");
        }
        // Match Android's flat composition; the 3DS foreground flag is for parallax.
        draw_background();
        const bool mask = (info[3] & 1) != 0;
        for (int pass = 0; pass < (mask ? 1 : 3); ++pass)
        for (const auto &segment : segments)
        {
            if (segment.screen >= count || !want_screen(segment.screen))
                continue;
            SDL_Rect source{segment.pos_tex[0], segment.pos_tex[1], segment.size_tex[0], segment.size_tex[1]};
            const auto dest = destination(offset[segment.screen].x + segment.pos_scr[0] / divisor,
                                          offset[segment.screen].y + segment.pos_scr[1] / divisor,
                                          segment.size_tex[0] / divisor, segment.size_tex[1] / divisor);
            if (!mask && pass == 0)
            {
                if (g_settings.segment_marking_alpha > 0)
                {
                    SDL_SetTextureAlphaMod(marking_.get(), g_settings.segment_marking_alpha);
                    copy_artwork(marking_.get(), source, dest);
                }
                continue;
            }
            if (!segment.buffer_state)
                continue;
            if (!mask && pass == 1)
            {
                SDL_FRect shadow = dest;
                shadow.x += 2 * zoom;
                shadow.y += 2 * zoom;
                SDL_SetTextureColorMod(atlas_.get(), 17, 17, 17);
                SDL_SetTextureAlphaMod(atlas_.get(), 0x18);
                copy_artwork(atlas_.get(), source, shadow);
                continue;
            }
            if (!mask)
            {
                const auto color = g_settings.background_color;
                SDL_SetTextureColorMod(atlas_.get(), Uint8(((color >> 16) & 255) * 0.12f),
                    Uint8(((color >> 8) & 255) * 0.12f), Uint8((color & 255) * 0.12f));
            }
            auto *texture = !mask && segment.color_index > 0 && segment.color_index < 5 ?
                colors_[segment.color_index].get() : atlas_.get();
            SDL_SetTextureAlphaMod(texture, 255);
            copy_artwork(texture, source, dest);
        }
    }
}
