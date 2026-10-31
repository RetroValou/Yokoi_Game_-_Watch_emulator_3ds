#pragma once

#include <cstdint>
#include <memory>
#include <map>
#include <vector>
#include "core/yokoi_audio.h"
#include "core/yokoi_ui.h"
#include "input/yokoi_controller.h"
#include "input/yokoi_pointer.h"
#include "render/yokoi_render.h"

class SM5XX;
class Virtual_Input;

namespace yokoi
{
    // Single-threaded game session and menu flow; SDL resources stay in their modules.
    class App
    {
    public:
        explicit App(SDL_Renderer *renderer, SDL_Renderer *secondary_renderer = nullptr);
        ~App();
        void initialize();
        void select(size_t selected);
        void start(bool resume);
        void start_fresh();
        void return_to_menu();
        bool handle_input(const ControllerState &state);
        void toggle_settings();
        void handle_pointer(PointerId id, float x, float y, PointerPhase phase, bool physical_bottom);
        void tick(uint32_t mask);
        void draw();
        void save();

        bool open_audio() { return audio_.open(); }
        bool audio_ready() const { return audio_.has_stream(); }
        bool in_menu() const { return ui_.mode != AppMode::game; }
        bool playing() const { return playing_; }
        const UiState &ui() const { return ui_; }
        uint8_t selected_index() const { return index_; }
        SM5XX *cpu() const { return cpu_.get(); }

    private:
        void update_segments();
        void navigate_game(int direction, uint8_t manufacturer);
        void navigate_manufacturer(int direction);
        void change_setting(int direction, bool activate);

        GameRenderer graphics_;
        std::unique_ptr<GameRenderer> secondary_graphics_;
        Audio audio_;
        const GW_rom *game_ = nullptr;
        uint8_t index_ = 0;
        std::unique_ptr<SM5XX> cpu_;
        std::unique_ptr<Virtual_Input> input_;
        std::vector<Segment> segments_;
        UiState ui_;
        uint32_t suppressed_inputs_ = 0;
        std::map<PointerId, int> pointers_;
        int gamea_pulse_ = 0;
        bool playing_ = false;
        uint32_t rate_accumulator_ = 0;
        int time_grace_ = 500;
    };
}
