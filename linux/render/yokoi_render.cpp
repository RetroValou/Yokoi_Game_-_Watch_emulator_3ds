#include "render/yokoi_render.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include "core/yokoi_sdl.h"
#include "std/settings.h"

namespace yokoi
{
    static SDL_Rect artwork_bounds(const SDL_FRect &area)
    {
        // Trim only canvas padding; round outward so edge pixels are retained.
        const int left = std::max(0, int(std::floor(area.x)));
        const int top = std::max(0, int(std::floor(area.y)));
        const int right = std::min(logical_width, int(std::ceil(area.x + area.w)));
        const int bottom = std::min(logical_height, int(std::ceil(area.y + area.h)));
        if (right <= left || bottom <= top)
            throw std::runtime_error("Invalid panel artwork dimensions");
        return SDL_Rect{left, top, right - left, bottom - top};
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
        if (requested_panel == Panel::combined)
        {
            if (!top_target_)
            {
                top_target_.reset(SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32,
                    SDL_TEXTUREACCESS_TARGET, logical_width, logical_height));
                require_sdl(top_target_ != nullptr, "Create top panel target");
            }
            if (!ui.top_only && !bottom_target_)
            {
                bottom_target_.reset(SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32,
                    SDL_TEXTUREACCESS_TARGET, logical_width, logical_height));
                require_sdl(bottom_target_ != nullptr, "Create bottom panel target");
            }
            auto *target = SDL_GetRenderTarget(renderer_);
            require_sdl(SDL_SetRenderTarget(renderer_, top_target_.get()) == 0, "Top panel target");
            const auto top_source = draw_panel(game, segments, ui, Panel::top);
            SDL_Rect bottom_source{};
            if (!ui.top_only)
            {
                require_sdl(SDL_SetRenderTarget(renderer_, bottom_target_.get()) == 0, "Bottom panel target");
                bottom_source = draw_panel(game, segments, ui, Panel::bottom);
            }
            require_sdl(SDL_SetRenderTarget(renderer_, target) == 0, "Restore panel target");
            int output_width, output_height, previous_width, previous_height;
            require_sdl(SDL_GetRendererOutputSize(renderer_, &output_width, &output_height) == 0,
                        "Get panel output size");
            SDL_RenderGetLogicalSize(renderer_, &previous_width, &previous_height);
            require_sdl(SDL_RenderSetLogicalSize(renderer_, 0, 0) == 0, "Use full panel output");
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderClear(renderer_);
            const auto fit_panel = [&](const SDL_Rect &source, int y, int available_height)
            {
                float width = float(output_width);
                float height = width * source.h / source.w;
                if (height > available_height)
                {
                    height = float(available_height);
                    width = height * source.w / source.h;
                }
                return SDL_FRect{(output_width - width) / 2, y + (available_height - height) / 2, width, height};
            };
            const int top_height = output_height / 2;
            auto top = fit_panel(top_source, 0, ui.top_only ? output_height : top_height);
            SDL_FRect bottom{};
            if (!ui.top_only)
            {
                bottom = fit_panel(bottom_source, top_height, output_height - top_height);
                if (ui.mode == AppMode::game && shared_lcd_reference(game, segments))
                {
                    top.y = (output_height - 2 * top.h) / 2;
                    bottom = top;
                    bottom.y += top.h;
                }
            }
            require_sdl(SDL_RenderCopyF(renderer_, top_target_.get(), &top_source, &top) == 0, "Copy top panel");
            if (!ui.top_only)
                require_sdl(SDL_RenderCopyF(renderer_, bottom_target_.get(), &bottom_source, &bottom) == 0, "Copy bottom panel");
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
            draw_panel(game, segments, ui, requested_panel);
        panel_ = requested_panel;
        if (settings_overlay)
            draw_settings(ui, !background_dimmed);
        SDL_RenderPresent(renderer_);
    }

    SDL_Rect GameRenderer::draw_panel(const GW_rom &game, const std::vector<Segment> &segments,
                                     const UiState &ui, Panel panel)
    {
        panel_ = panel;
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        if (ui.mode == AppMode::game)
            return draw_game(game, segments, !ui.top_only);
        else if (panel == Panel::bottom)
            return draw_console(game);
        else
            draw_menu(game, ui);
        return SDL_Rect{0, 0, logical_width, logical_height};
    }

    void GameRenderer::draw_menu(const GW_rom &game, const UiState &ui)
    {
        const auto centered = [&](const std::string &text, float baseline, float scale, uint32_t color)
        {
            font_.draw((logical_width - text_width(text) * scale) / 2,
                      baseline - 10 * scale, text, scale, color);
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
            font_.draw(160 - text_width(left) * scale / 2, 298, left, scale, 0xcccccc);
            font_.draw(480 - text_width(right) * scale / 2, 298, right, scale, 0xcccccc);
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
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
    }

    SDL_Rect GameRenderer::draw_console(const GW_rom &game)
    {
        if (!console_ || !game.console_info)
            return SDL_Rect{0, 0, logical_width, logical_height};
        const auto *info = game.console_info;
        SDL_Rect source{info[2], info[3], info[4], info[5]};
        const float scale = std::min(float(logical_width) / std::max(1, source.w),
                                     float(logical_height) / std::max(1, source.h));
        SDL_FRect destination{(logical_width - source.w * scale) / 2,
                              (logical_height - source.h * scale) / 2,
                              source.w * scale, source.h * scale};
        copy_texture(renderer_, console_.get(), source, destination);
        return artwork_bounds(destination);
    }

    SDL_Rect GameRenderer::draw_game(const GW_rom &game, const std::vector<Segment> &segments, bool match_lcd_sizes)
    {
        const auto *info = game.segment_info;
        const float divisor = std::max(1, int(info[2]));
        int count = 1;
        for (const auto &segment : segments)
            if (segment.screen == 1)
                count = 2;
        float width[2] = {info[4] / divisor, info[4] / divisor};
        float height[2] = {info[5] / divisor, info[5] / divisor};
        if (count == 2)
        {
            width[1] = info[6] / divisor;
            height[1] = info[7] / divisor;
        }
        // Preserve both screens' aspect ratios in a side-by-side or stacked canvas.
        const bool horizontal = (info[3] & 2) != 0;
        // Android/3DS keep horizontal games together; only clamshell games split.
        const bool split = panel_ != Panel::combined && count == 2 && !horizontal;
        if (panel_ == Panel::bottom && !split)
            return draw_console(game);
        const int selected_screen = panel_ == Panel::bottom ? 1 : 0;
        const auto shared_screen = split && match_lcd_sizes ? shared_lcd_reference(game, segments) : std::nullopt;
        const int sizing_screen = shared_screen.value_or(selected_screen);
        const float canvas_width = split ? width[sizing_screen] : count == 1 ? width[0] :
            horizontal ? width[0] + width[1] : std::max(width[0], width[1]);
        const float canvas_height = split ? height[sizing_screen] : count == 1 ? height[0] :
            horizontal ? std::max(height[0], height[1]) : height[0] + height[1];
        const float zoom = std::min(logical_width / std::max(1.f, canvas_width),
                                    logical_height / std::max(1.f, canvas_height));
        float offset_x[2] = {(canvas_width - width[0]) / 2, (canvas_width - width[1]) / 2};
        float offset_y[2] = {0, height[0]};
        if (horizontal && count == 2)
        {
            offset_x[0] = 0;
            offset_x[1] = width[0];
            offset_y[0] = (canvas_height - height[0]) / 2;
            offset_y[1] = (canvas_height - height[1]) / 2;
        }
        if (split)
        {
            offset_x[0] = offset_x[1] = 0;
            offset_y[0] = offset_y[1] = 0;
        }
        const auto want_screen = [&](int screen) { return !split || screen == selected_screen; };
        const auto destination = [&](float x, float y, float w, float h)
        {
            if (shared_screen)
            {
                x = x / width[selected_screen] * canvas_width;
                y = y / height[selected_screen] * canvas_height;
                w = w / width[selected_screen] * canvas_width;
                h = h / height[selected_screen] * canvas_height;
            }
            return SDL_FRect{(logical_width - canvas_width * zoom) / 2 + x * zoom,
                             (logical_height - canvas_height * zoom) / 2 + y * zoom,
                             w * zoom, h * zoom};
        };
        const bool background_in_front = game.background_info && game.background_info[3 + count * 4] != 0;
        const auto draw_background = [&]
        {
            if (!background_ || !game.background_info)
                return;
            for (int screen = 0; screen < count; ++screen)
            {
                if (!want_screen(screen))
                    continue;
                const auto *background = game.background_info + 2 + screen * 4;
                SDL_Rect source{background[0], background[1], background[2], background[3]};
                const auto dest = destination(offset_x[screen], offset_y[screen], width[screen], height[screen]);
                copy_texture(renderer_, background_.get(), source, dest);
            }
        };
        for (int screen = 0; screen < count; ++screen)
        {
            if (!want_screen(screen))
                continue;
            const auto dest = destination(offset_x[screen], offset_y[screen], width[screen], height[screen]);
            const uint32_t color = (info[3] & 1) ? 0x080908 : g_settings.background_color;
            SDL_SetRenderDrawColor(renderer_, (color >> 16) & 255, (color >> 8) & 255, color & 255, 255);
            SDL_RenderFillRectF(renderer_, &dest);
        }
        // Artwork can be behind the LCD segments or overlay them (mask packs).
        if (!background_in_front)
            draw_background();
        const bool mask = (info[3] & 1) != 0;
        for (int pass = 0; pass < (mask ? 1 : 3); ++pass)
        for (const auto &segment : segments)
        {
            if (segment.screen >= count || !want_screen(segment.screen))
                continue;
            SDL_Rect source{segment.pos_tex[0], segment.pos_tex[1], segment.size_tex[0], segment.size_tex[1]};
            const auto dest = destination(offset_x[segment.screen] + segment.pos_scr[0] / divisor,
                                          offset_y[segment.screen] + segment.pos_scr[1] / divisor,
                                          segment.size_tex[0] / divisor, segment.size_tex[1] / divisor);
            if (!mask && pass == 0)
            {
                if (g_settings.segment_marking_alpha > 0)
                {
                    SDL_SetTextureAlphaMod(marking_.get(), g_settings.segment_marking_alpha);
                    copy_texture(renderer_, marking_.get(), source, dest);
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
                copy_texture(renderer_, atlas_.get(), source, shadow);
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
            copy_texture(renderer_, texture, source, dest);
        }
        if (background_in_front)
            draw_background();
        return artwork_bounds(SDL_FRect{(logical_width - canvas_width * zoom) / 2,
                                         (logical_height - canvas_height * zoom) / 2,
                                         canvas_width * zoom, canvas_height * zoom});
    }
}
