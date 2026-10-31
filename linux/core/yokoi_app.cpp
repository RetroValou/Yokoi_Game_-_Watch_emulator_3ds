#include "core/yokoi_app.h"

#include <atomic>
#include <cstdio>
#include <stdexcept>
#include "std/gw_pack.h"
#include "std/savestate.h"
#include "virtual_i_o/virtual_input.h"
#include "yokoi_cpu_utils.h"
#include "yokoi_input_mapping.h"

namespace yokoi
{
    App::App(SDL_Renderer *renderer) : graphics_(renderer) {}
    App::~App() = default;

    void App::save()
    {
        if (cpu_ && playing_ && !save_game_state(cpu_.get(), index_))
            std::fprintf(stderr, "Save failed for %s\n", game_->ref.c_str());
    }

    void App::select(size_t selected)
    {
        save();
        playing_ = false;
        menu_ = true;
        input_.reset();
        cpu_.reset();
        index_ = uint8_t(selected);
        game_ = load_game(index_);
        if (!game_ || !game_->segment_info)
            throw std::runtime_error("Missing game metadata");
        graphics_.load(*game_);
        segments_.assign(game_->segment, game_->segment + game_->size_segment);
    }

    void App::start(bool resume)
    {
        if (!game_)
            throw std::runtime_error("No game selected");
        if (!cpu_)
        {
            if (!yokoi_cpu_create_instance(cpu_, game_->rom, uint16_t(game_->size_rom)))
                throw std::runtime_error("Unsupported CPU/ROM size");
            cpu_->init();
            cpu_->load_rom(game_->rom, game_->size_rom);
            cpu_->load_rom_melody(game_->melody, game_->size_melody);
            cpu_->load_rom_time_addresses(game_->ref);
            cpu_->time_set(false);
            yokoi_cpu_set_time_if_needed(cpu_.get());
            input_.reset(get_input_config(cpu_.get(), game_->ref));
            if (input_)
                cpu_->set_input_multiplexage(input_->use_multiplexage);
            // Let the ROM finish booting before setting its clock/restoring a save.
            for (int i = 0; i < 2000; ++i)
                cpu_->step();
            cpu_->time_set(false);
            yokoi_cpu_set_time_if_needed(cpu_.get());
            if (resume && save_state_exists(index_) && !load_game_state(cpu_.get(), index_))
                std::fprintf(stderr, "Could not load save; starting fresh\n");
            rate_accumulator_ = 0;
            audio_.reset_sampling();
            time_grace_ = 500;
        }
        audio_.configure(*cpu_);
        menu_ = false;
        playing_ = true;
    }

    void App::start_fresh()
    {
        input_.reset();
        cpu_.reset();
        start(false);
    }

    void App::return_to_menu()
    {
        save();
        menu_ = true;
        audio_.clear_queue();
    }

    bool App::handle_input(const ControllerState &state)
    {
        if ((state.held & (CTL_START | CTL_SELECT)) == (CTL_START | CTL_SELECT))
            return true;
        if (menu_)
        {
            if (state.pressed & CTL_DPAD_LEFT)
                select((index_ + gw_pack::game_count() - 1) % gw_pack::game_count());
            if (state.pressed & CTL_DPAD_RIGHT)
                select((index_ + 1) % gw_pack::game_count());
            if (state.pressed & CTL_A)
                start(true);
            if (state.pressed & CTL_X)
                start_fresh();
            if ((state.pressed & CTL_B) && playing_)
                start(true);
        }
        else if ((state.pressed & CTL_B) && (state.held & CTL_L1))
            return_to_menu();
        return false;
    }

    void App::update_segments()
    {
        // Match the existing blink protection: visible segments span adjacent updates.
        for (auto &segment : segments_)
        {
            const bool next = cpu_->get_segments_state(segment.id[0], segment.id[1], segment.id[2]);
            const bool visible = (segment.state && (next || segment.buffer_state)) ||
                                 (next && segment.buffer_state);
            segment.buffer_state = visible;
            segment.state = next;
        }
        cpu_->segments_state_are_update = false;
    }

    void App::tick(uint32_t mask)
    {
        if (menu_ || !cpu_)
            return;
        std::atomic<int> pulse{0};
        yokoi_input_apply_controller_mask(input_.get(), mask, 0, pulse, pulse);
        // Carry fractional CPU cycles between 60 Hz frames instead of rounding away time.
        rate_accumulator_ += cpu_->frequency;
        uint32_t steps = rate_accumulator_ / 60;
        rate_accumulator_ %= 60;
        audio_.begin_frame();
        while (steps--)
        {
            if (cpu_->step())
            {
                // Keep synchronizing the clock during the initial display updates.
                if (time_grace_ > 0)
                {
                    --time_grace_;
                    cpu_->time_set(false);
                    yokoi_cpu_set_time_if_needed(cpu_.get());
                }
                if (cpu_->segments_state_are_update)
                    update_segments();
            }
            audio_.sample(*cpu_);
        }
        audio_.submit_frame();
    }

    void App::draw()
    {
        graphics_.draw(*game_, segments_, menu_);
    }
}
