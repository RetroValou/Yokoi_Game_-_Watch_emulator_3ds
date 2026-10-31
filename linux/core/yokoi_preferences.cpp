#include "core/yokoi_ui.h"

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include "std/platform_paths.h"

namespace yokoi
{
    void load_display_preferences(UiState &ui)
    {
        ui.swap_screens = false;
        const auto path = storage_path("yokoi_linux_preferences.dat");
        FILE *file = std::fopen(path.c_str(), "rb");
        if (!file)
        {
            if (errno != ENOENT)
                throw std::runtime_error("Cannot read display preferences: " + path + ": " + std::strerror(errno));
            return;
        }
        std::array<unsigned char, 6> data{};
        const auto read = std::fread(data.data(), 1, data.size(), file);
        const bool extra = std::fgetc(file) != EOF;
        const bool failed = std::ferror(file) != 0;
        const bool closed = std::fclose(file) == 0;
        if (failed || !closed)
            throw std::runtime_error("Cannot read display preferences: " + path);
        // Accept retired YLP2 files, preserving screen swap but ignoring face-button swap.
        const bool supported = std::memcmp(data.data(), "YLP1", 4) == 0 ||
                               std::memcmp(data.data(), "YLP2", 4) == 0;
        if (read != data.size() || extra || !supported ||
            data[4] > 1 || data[5] > 1)
        {
            std::fprintf(stderr, "Invalid display preferences: %s; using defaults.\n", path.c_str());
            return;
        }
        ui.swap_screens = data[4] != 0;
    }

    void save_display_preferences(const UiState &ui)
    {
        const auto path = storage_path("yokoi_linux_preferences.dat");
        const auto temporary = path + ".tmp";
        const std::array<unsigned char, 6> data{'Y', 'L', 'P', '1',
            static_cast<unsigned char>(ui.swap_screens), 0};
        FILE *file = std::fopen(temporary.c_str(), "wb");
        if (!file)
            throw std::runtime_error("Cannot save display preferences: " + temporary + ": " + std::strerror(errno));
        const bool wrote = std::fwrite(data.data(), 1, data.size(), file) == data.size();
        const bool closed = std::fclose(file) == 0;
        if (!wrote || !closed || std::rename(temporary.c_str(), path.c_str()) != 0)
        {
            if (std::remove(temporary.c_str()) != 0)
                std::fprintf(stderr, "Cannot remove failed preferences write: %s: %s\n",
                             temporary.c_str(), std::strerror(errno));
            throw std::runtime_error("Cannot save display preferences: " + path);
        }
    }
}
