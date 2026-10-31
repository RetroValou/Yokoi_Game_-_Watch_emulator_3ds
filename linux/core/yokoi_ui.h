#pragma once

#include <array>
#include <cstddef>
#include <vector>

namespace yokoi
{
    enum class AppMode { select, load_prompt, game };
    enum class Setting { segments, background, swap, quit_game, top_only, close };

    struct UiState
    {
        AppMode mode = AppMode::select;
        bool settings_open = false;
        std::size_t settings_row = 0;
        bool load_save = false;
        bool has_save = false;
        bool swap_screens = false;
        bool top_only = false;
    };

    inline constexpr std::array<int, 3> segment_alphas{0, 5, 10};
    inline constexpr std::array<const char *, 3> segment_names{"None", "Light", "Dark"};

    inline std::vector<Setting> settings_items(const UiState &ui, bool dual)
    {
        std::vector<Setting> items{Setting::segments, Setting::background};
        if (dual)
            items.push_back(Setting::swap);
        if (ui.mode == AppMode::game)
        {
            items.push_back(Setting::quit_game);
            if (!dual)
                items.push_back(Setting::top_only);
        }
        items.push_back(Setting::close);
        return items;
    }

    void load_display_preferences(UiState &ui);
    void save_display_preferences(const UiState &ui);
}
