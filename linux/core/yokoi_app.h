#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "core/yokoi_audio.h"
#include "input/yokoi_controller.h"
#include "render/yokoi_render.h"

class SM5XX;
class Virtual_Input;

namespace yokoi
{
    // Single-threaded game session and menu flow; SDL resources stay in their modules.
    class App
    {
    public:
        explicit App(SDL_Renderer *renderer);
        ~App();
        void select(size_t selected);
        void start(bool resume);
        void start_fresh();
        void return_to_menu();
        bool handle_input(const ControllerState &state);
        void tick(uint32_t mask);
        void draw();
        void save();

        bool open_audio() { return audio_.open(); }
        bool audio_ready() const { return audio_.has_stream(); }
        bool in_menu() const { return menu_; }
        bool playing() const { return playing_; }
        uint8_t selected_index() const { return index_; }
        SM5XX *cpu() const { return cpu_.get(); }

    private:
        void update_segments();

        GameRenderer graphics_;
        Audio audio_;
        const GW_rom *game_ = nullptr;
        uint8_t index_ = 0;
        std::unique_ptr<SM5XX> cpu_;
        std::unique_ptr<Virtual_Input> input_;
        std::vector<Segment> segments_;
        bool menu_ = true;
        bool playing_ = false;
        uint32_t rate_accumulator_ = 0;
        int time_grace_ = 500;
    };
}
