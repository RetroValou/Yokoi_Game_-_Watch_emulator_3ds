#include "core/yokoi_app.h"

#include <atomic>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <stdexcept>
#include "std/gw_pack.h"
#include "std/savestate.h"
#include "std/settings.h"
#include "virtual_i_o/virtual_input.h"
#include "virtual_i_o/time_addresses.h"
#include "yokoi_cpu_utils.h"
#include "yokoi_input_mapping.h"

namespace yokoi
{
    App::App(SDL_Renderer *renderer, SDL_Renderer *secondary_renderer)
        : graphics_(renderer, secondary_renderer ? Panel::top : Panel::combined),
          secondary_graphics_(secondary_renderer ?
              std::make_unique<GameRenderer>(secondary_renderer, Panel::bottom) : nullptr) {}
    App::~App() = default;

    void App::initialize()
    {
        load_settings();
        load_display_preferences(ui_);
        uint8_t selected = 0;
        const auto manufacturer = load_last_selected_manufacturer();
        if (!try_load_last_game_index_for_manufacturer(manufacturer, &selected) ||
            !load_game(selected) || load_game(selected)->manufacturer != manufacturer)
        {
            selected = 0;
            for (size_t i = 0; i < gw_pack::game_count(); ++i)
                if (get_ref(uint8_t(i)) == "JR_55")
                {
                    selected = uint8_t(i);
                    break;
                }
        }
        select(selected);
    }

    void App::save()
    {
        if (cpu_ && playing_ && !save_game_state(cpu_.get(), index_))
            std::fprintf(stderr, "Save failed for %s\n", game_->ref.c_str());
    }

    void App::select(size_t selected)
    {
        if (selected >= gw_pack::game_count())
            throw std::runtime_error("Invalid selected game index");
        save();
        playing_ = false;
        ui_.mode = AppMode::select;
        ui_.settings_open = false;
        ui_.top_only = false;
        pointers_.clear();
        gamea_pulse_ = 0;
        input_.reset();
        cpu_.reset();
        index_ = uint8_t(selected);
        game_ = load_game(index_);
        if (!game_ || !game_->segment_info)
            throw std::runtime_error("Missing game metadata");
        if (game_->manufacturer >= GW_rom::MANUFACTURER_COUNT)
            throw std::runtime_error("Invalid game manufacturer");
        graphics_.load(*game_);
        if (secondary_graphics_)
            secondary_graphics_->load(*game_);
        segments_.assign(game_->segment, game_->segment + game_->size_segment);
        ui_.has_save = save_state_exists(index_);
        ui_.load_save = ui_.has_save;
        if (!save_last_selected_game(game_->manufacturer, game_->ref))
            throw std::runtime_error("Cannot save last selected game");
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
            // Startup can overwrite clock RAM before copying it to the display.
            int warmup_time_grace = 500;
            for (int i = 0; i < 2000; ++i)
                if (cpu_->step() && warmup_time_grace > 0)
                {
                    --warmup_time_grace;
                    cpu_->time_set(false);
                    yokoi_cpu_set_time_if_needed(cpu_.get());
                }
            if (resume && save_state_exists(index_) && !load_game_state(cpu_.get(), index_))
                std::fprintf(stderr, "Could not load save; starting fresh\n");
            cpu_->time_set(false);
            yokoi_cpu_set_time_if_needed(cpu_.get());
            if (get_time_addresses(game_->ref))
            {
                const auto now = std::time(nullptr);
                const auto *local = std::localtime(&now);
                char clock[64];
                if (local && std::strftime(clock, sizeof(clock), "%Y-%m-%d %H:%M:%S %Z", local))
                    std::fprintf(stderr, "Automatic clock for %s: %s (device local time).\n",
                                 game_->ref.c_str(), clock);
                else
                    std::fprintf(stderr, "Automatic clock unavailable: cannot read device local time.\n");
            }
            else
                std::fprintf(stderr, "Automatic clock unavailable for %s: no clock RAM mapping.\n",
                             game_->ref.c_str());
            rate_accumulator_ = 0;
            audio_.reset_sampling();
            time_grace_ = 500;
        }
        audio_.configure(*cpu_);
        ui_.mode = AppMode::game;
        ui_.settings_open = false;
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
        playing_ = false;
        input_.reset();
        cpu_.reset();
        pointers_.clear();
        gamea_pulse_ = 0;
        ui_.mode = AppMode::select;
        ui_.settings_open = false;
        ui_.top_only = false;
        ui_.has_save = save_state_exists(index_);
        ui_.load_save = ui_.has_save;
        audio_.clear_queue();
    }

    void App::navigate_game(int direction, uint8_t manufacturer)
    {
        const int count = int(gw_pack::game_count());
        for (int step = 1; step <= count; ++step)
        {
            const auto index = uint8_t((int(index_) + direction * step + count) % count);
            const auto *game = load_game(index);
            if (!game)
                throw std::runtime_error("Missing game during navigation");
            if (game->manufacturer == manufacturer)
            {
                select(index);
                return;
            }
        }
    }

    void App::navigate_manufacturer(int direction)
    {
        for (int step = 1; step < GW_rom::MANUFACTURER_COUNT; ++step)
        {
            const auto manufacturer = uint8_t((game_->manufacturer + direction * step +
                                               GW_rom::MANUFACTURER_COUNT) % GW_rom::MANUFACTURER_COUNT);
            bool available = false;
            for (size_t i = 0; i < gw_pack::game_count(); ++i)
            {
                const auto *game = load_game(uint8_t(i));
                if (!game)
                    throw std::runtime_error("Missing game during manufacturer navigation");
                available |= game->manufacturer == manufacturer;
            }
            if (!available)
                continue;
            uint8_t saved = 0;
            if (try_load_last_game_index_for_manufacturer(manufacturer, &saved) &&
                load_game(saved) && load_game(saved)->manufacturer == manufacturer)
                select(saved);
            else
                navigate_game(direction, manufacturer);
            return;
        }
    }

    void App::toggle_settings()
    {
        ui_.settings_open = !ui_.settings_open;
        ui_.settings_row = 0;
        pointers_.clear();
        gamea_pulse_ = 0;
        audio_.clear_queue();
    }

    void App::change_setting(int direction, bool activate)
    {
        const auto items = settings_items(ui_, secondary_graphics_ != nullptr);
        switch (items.at(ui_.settings_row))
        {
        case Setting::segments:
        {
            int index = 1;
            for (size_t i = 0; i < segment_alphas.size(); ++i)
                if (g_settings.segment_marking_alpha == segment_alphas[i])
                    index = int(i);
            g_settings.segment_marking_alpha = uint8_t(segment_alphas[(index + direction + 3) % 3]);
            if (!save_settings())
                throw std::runtime_error("Cannot save segment settings");
            break;
        }
        case Setting::background:
        {
            int index = 0;
            for (int i = 0; i < NUM_BG_PRESETS; ++i)
                if (g_settings.background_color == BACKGROUND_PRESETS[i].color)
                    index = i;
            g_settings.background_color = BACKGROUND_PRESETS[(index + direction + NUM_BG_PRESETS) % NUM_BG_PRESETS].color;
            if (!save_settings())
                throw std::runtime_error("Cannot save background settings");
            break;
        }
        case Setting::controller_layout:
            ui_.controller_layout = ui_.controller_layout == ControllerLayout::standard ?
                ControllerLayout::rgds : ControllerLayout::standard;
            save_display_preferences(ui_);
            break;
        case Setting::swap:
            ui_.swap_screens = !ui_.swap_screens;
            pointers_.clear();
            save_display_preferences(ui_);
            break;
        case Setting::quit_game:
            if (activate)
                return_to_menu();
            break;
        case Setting::top_only:
            if (activate)
            {
                ui_.top_only = !ui_.top_only;
                toggle_settings();
            }
            break;
        case Setting::exit_app:
            if (activate)
                quit_requested_ = true;
            break;
        case Setting::close:
            if (activate)
                toggle_settings();
            break;
        }
    }

    bool App::handle_input(const ControllerState &state)
    {
        if (quit_requested_ || (state.held & (CTL_START | CTL_SELECT)) == (CTL_START | CTL_SELECT))
            return true;
        if (state.pressed & CTL_MENU)
        {
            toggle_settings();
            suppressed_inputs_ |= state.held;
            return quit_requested_;
        }
        if (ui_.settings_open)
        {
            const auto count = settings_items(ui_, secondary_graphics_ != nullptr).size();
            if (state.pressed & CTL_B)
                toggle_settings();
            else if (state.pressed & CTL_DPAD_UP)
                ui_.settings_row = (ui_.settings_row + count - 1) % count;
            else if (state.pressed & CTL_DPAD_DOWN)
                ui_.settings_row = (ui_.settings_row + 1) % count;
            else if (state.pressed & CTL_DPAD_LEFT)
                change_setting(-1, false);
            else if (state.pressed & (CTL_DPAD_RIGHT | CTL_A))
                change_setting(1, (state.pressed & CTL_A) != 0);
            suppressed_inputs_ |= state.held;
            return quit_requested_;
        }
        if (ui_.mode == AppMode::select)
        {
            if (state.pressed & CTL_DPAD_LEFT) navigate_game(-1, game_->manufacturer);
            if (state.pressed & CTL_DPAD_RIGHT) navigate_game(1, game_->manufacturer);
            if (state.pressed & CTL_DPAD_UP) navigate_manufacturer(1);
            if (state.pressed & CTL_DPAD_DOWN) navigate_manufacturer(-1);
            if (state.pressed & CTL_A)
            {
                ui_.has_save = save_state_exists(index_);
                ui_.load_save = ui_.has_save;
                ui_.mode = AppMode::load_prompt;
            }
        }
        else if (ui_.mode == AppMode::load_prompt)
        {
            if (state.pressed & CTL_B)
                ui_.mode = AppMode::select;
            else if (state.pressed & (CTL_DPAD_LEFT | CTL_DPAD_RIGHT))
                ui_.load_save = !ui_.load_save && save_state_exists(index_);
            else if (state.pressed & CTL_A)
            {
                start(ui_.load_save);
                suppressed_inputs_ |= state.held;
            }
        }
        else if ((state.pressed & CTL_B) && (state.held & CTL_L1))
            return_to_menu();
        return false;
    }

    void App::handle_pointer(PointerId id, float x, float y, PointerPhase phase, bool physical_bottom)
    {
        if (quit_requested_)
            return;
        if (phase == PointerPhase::release)
        {
            pointers_.erase(id);
            return;
        }
        const bool dual = secondary_graphics_ != nullptr;
        if (dual && (ui_.settings_open ? !physical_bottom :
                     physical_bottom == ui_.swap_screens))
            return;
        if (ui_.settings_open)
        {
            if (phase != PointerPhase::press)
                return;
            const auto items = settings_items(ui_, dual);
            if (x >= 64 && x < 576 && y >= 78 && y < 78 + items.size() * 50)
            {
                ui_.settings_row = size_t((y - 78) / 50);
                change_setting(1, true);
            }
            return;
        }
        if (ui_.mode == AppMode::select && phase == PointerPhase::press)
        {
            if (y >= logical_height / 2)
            {
                if (x < logical_width / 3) navigate_game(-1, game_->manufacturer);
                else if (x > logical_width * 2 / 3) navigate_game(1, game_->manufacturer);
                else handle_input({CTL_A, CTL_A});
            }
            else
            {
                if (x < logical_width / 3) navigate_manufacturer(1);
                else if (x > logical_width * 2 / 3) navigate_manufacturer(-1);
            }
        }
        else if (ui_.mode == AppMode::load_prompt && phase == PointerPhase::press)
        {
            if (y >= logical_height / 2)
                start(x >= logical_width / 2 && save_state_exists(index_));
        }
        else if (ui_.mode == AppMode::game)
        {
            if (phase == PointerPhase::press && y >= logical_height / 2 &&
                x >= logical_width / 3 && x <= logical_width * 2 / 3)
            {
                pointers_.erase(id);
                gamea_pulse_ = 8;
            }
            else
                pointers_[id] = x < logical_width / 2 ? 1 : 2;
        }
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
        int actions = 0;
        for (const auto &[id, value] : pointers_)
            actions |= value;
        suppressed_inputs_ &= mask;
        if (in_menu() || ui_.settings_open || !cpu_)
            return;
        mask &= ~suppressed_inputs_;
        std::atomic<int> gamea{gamea_pulse_}, gameb{0};
        yokoi_input_apply_controller_mask(input_.get(), mask, actions, gamea, gameb);
        if (gamea_pulse_ > 0)
            --gamea_pulse_;
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
        const bool dual = secondary_graphics_ != nullptr;
        graphics_.set_panel(dual ? (ui_.swap_screens ? Panel::bottom : Panel::top) : Panel::combined);
        graphics_.draw(*game_, segments_, ui_, !dual && ui_.settings_open);
        if (secondary_graphics_)
        {
            secondary_graphics_->set_panel(ui_.swap_screens ? Panel::top : Panel::bottom);
            secondary_graphics_->draw(*game_, segments_, ui_, ui_.settings_open);
        }
    }
}
