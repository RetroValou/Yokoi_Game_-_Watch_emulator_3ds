#include "render/yokoi_render.h"

#include <algorithm>

namespace yokoi
{
    void GameRenderer::load(const GW_rom &game)
    {
        const bool mask_atlas = (game.segment_info[3] & 1) != 0;
        atlas_ = load_texture(renderer_, game.path_segment, mask_atlas ? -1 : 0x080908);
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
        SDL_SetRenderDrawColor(renderer_, 22, 27, 33, 255);
        SDL_RenderClear(renderer_);
        if (menu)
            draw_menu(game);
        else
            draw_game(game, segments);
        SDL_RenderPresent(renderer_);
    }

    void GameRenderer::draw_menu(const GW_rom &game)
    {
        draw_text(renderer_, 24, 24, "YOKOI - GAME & WATCH");
        draw_text(renderer_, 24, 64, game.name, text_width(game.name) > 290 ? 1 : 2);
        draw_text(renderer_, 24, 96, game.ref + "  " + game.date);
        if (console_ && game.console_info)
        {
            const auto *info = game.console_info;
            SDL_Rect source{info[2], info[3], info[4], info[5]};
            const float scale = std::min(560.f / std::max(1, source.w), 215.f / std::max(1, source.h));
            SDL_FRect destination{(logical_width - source.w * scale) / 2, 145,
                                  source.w * scale, source.h * scale};
            copy_texture(renderer_, console_.get(), source, destination);
        }
        draw_text(renderer_, 24, 375, "LEFT/RIGHT: CHOOSE   A: PLAY/RESUME");
        draw_text(renderer_, 24, 405, "X: NEW GAME   B: RESUME");
        draw_text(renderer_, 24, 435, "START: GAME A   SELECT: GAME B");
        draw_text(renderer_, 24, 460, "SELECT + START: SAVE AND EXIT", 1);
    }

    void GameRenderer::draw_game(const GW_rom &game, const std::vector<Segment> &segments)
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
        const float canvas_width = count == 1 ? width[0] :
            horizontal ? width[0] + width[1] : std::max(width[0], width[1]);
        const float canvas_height = count == 1 ? height[0] :
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
        const auto destination = [&](float x, float y, float w, float h)
        {
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
                const auto *background = game.background_info + 2 + screen * 4;
                SDL_Rect source{background[0], background[1], background[2], background[3]};
                const auto dest = destination(offset_x[screen], offset_y[screen], width[screen], height[screen]);
                copy_texture(renderer_, background_.get(), source, dest);
            }
        };
        for (int screen = 0; screen < count; ++screen)
        {
            const auto dest = destination(offset_x[screen], offset_y[screen], width[screen], height[screen]);
            SDL_SetRenderDrawColor(renderer_, 195, 200, 178, 255);
            SDL_RenderFillRectF(renderer_, &dest);
        }
        // Artwork can be behind the LCD segments or overlay them (mask packs).
        if (!background_in_front)
            draw_background();
        const bool mask = (info[3] & 1) != 0;
        for (const auto &segment : segments)
        {
            if (segment.screen >= count || (!segment.buffer_state && mask))
                continue;
            SDL_Rect source{segment.pos_tex[0], segment.pos_tex[1], segment.size_tex[0], segment.size_tex[1]};
            const auto dest = destination(offset_x[segment.screen] + segment.pos_scr[0] / divisor,
                                          offset_y[segment.screen] + segment.pos_scr[1] / divisor,
                                          segment.size_tex[0] / divisor, segment.size_tex[1] / divisor);
            auto *texture = !mask && segment.color_index > 0 && segment.color_index < 5 ?
                colors_[segment.color_index].get() : atlas_.get();
            SDL_SetTextureAlphaMod(texture, segment.buffer_state ? 255 : 12);
            copy_texture(renderer_, texture, source, dest);
        }
        if (background_in_front)
            draw_background();
    }
}
