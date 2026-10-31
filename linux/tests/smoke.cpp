#include <SDL.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <stdexcept>
#include <string>
#include "core/yokoi_app.h"
#include "core/yokoi_runtime.h"
#include "input/yokoi_controller.h"
#include "render/yokoi_graphics.h"
#include "std/gw_pack.h"
#include "std/platform_paths.h"
#include "std/savestate.h"
#include "std/settings.h"
#include "SM5XX/SM5XX.h"
#include "yokoi_input_mapping.h"

using namespace yokoi;

extern "C" SDL_AudioDeviceID __real_SDL_OpenAudioDevice(
    const char *, int, const SDL_AudioSpec *, SDL_AudioSpec *, int);
extern "C" SDL_AudioStream *__real_SDL_NewAudioStream(
    SDL_AudioFormat, Uint8, int, SDL_AudioFormat, Uint8, int);
extern "C" int __real_SDL_GetNumVideoDisplays();
extern "C" SDL_Window *__real_SDL_CreateWindow(const char *, int, int, int, int, Uint32);
extern "C" int __real_SDL_GetWindowDisplayIndex(SDL_Window *);
extern "C" SDL_Renderer *__real_SDL_CreateRenderer(SDL_Window *, int, Uint32);
extern "C" int __real_SDL_RenderSetLogicalSize(SDL_Renderer *, int, int);
extern "C" void __real_SDL_RenderPresent(SDL_Renderer *);
extern "C" std::time_t __real_time(std::time_t *);
static std::time_t test_time = 0;
static int iconv_calls = 0;
static SDL_Surface *captured_frame = nullptr;

extern "C" char *__wrap_SDL_iconv_string(const char *, const char *, const char *, size_t)
{
    ++iconv_calls;
    SDL_SetError("Test firmware has no character conversion support");
    return nullptr;
}

extern "C" std::time_t __wrap_time(std::time_t *result)
{
    if (!test_time)
        return __real_time(result);
    if (result)
        *result = test_time;
    return test_time;
}

enum class DisplayFailure { none, window, placement, renderer, logical_size };
static struct
{
    int count = -1;
    DisplayFailure failure = DisplayFailure::none;
    bool swap_screens = false;
    SDL_Window *primary = nullptr;
    SDL_Window *secondary = nullptr;
    int window_attempts = 0;
    int primary_frames = 0;
    int secondary_frames = 0;
} displays;

extern "C" int __wrap_SDL_GetNumVideoDisplays()
{
    if (displays.count < 0)
        return __real_SDL_GetNumVideoDisplays();
    if (displays.count == 0)
        SDL_SetError("Test device has no displays");
    return displays.count;
}

// The dummy backend has one display; emulate placement but use real SDL windows.
extern "C" SDL_Window *__wrap_SDL_CreateWindow(
    const char *title, int x, int y, int width, int height, Uint32 flags)
{
    if (displays.count < 0)
        return __real_SDL_CreateWindow(title, x, y, width, height, flags);
    assert(SDL_WINDOWPOS_ISCENTERED(x) && x == y);
    const int index = x & 0xffff;
    assert(index == 0 || index == 1);
    ++displays.window_attempts;
    if (index == 1 && displays.failure == DisplayFailure::window)
    {
        SDL_SetError("Test second window unavailable");
        return nullptr;
    }
    auto *window = __real_SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED_DISPLAY(0),
                                           SDL_WINDOWPOS_CENTERED_DISPLAY(0), width, height, flags);
    (index == 0 ? displays.primary : displays.secondary) = window;
    return window;
}

extern "C" int __wrap_SDL_GetWindowDisplayIndex(SDL_Window *window)
{
    if (displays.count >= 0 && window == displays.secondary)
        return displays.failure == DisplayFailure::placement ? 0 : 1;
    return __real_SDL_GetWindowDisplayIndex(window);
}

extern "C" SDL_Renderer *__wrap_SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags)
{
    if (displays.count >= 0 && window == displays.secondary &&
        displays.failure == DisplayFailure::renderer)
    {
        SDL_SetError("Test second renderer unavailable");
        return nullptr;
    }
    return __real_SDL_CreateRenderer(window, index, flags);
}

extern "C" int __wrap_SDL_RenderSetLogicalSize(SDL_Renderer *renderer, int width, int height)
{
    if (displays.count >= 0 && SDL_RenderGetWindow(renderer) == displays.secondary &&
        displays.failure == DisplayFailure::logical_size)
        return SDL_SetError("Test second logical size unavailable");
    return __real_SDL_RenderSetLogicalSize(renderer, width, height);
}

extern "C" void __wrap_SDL_RenderPresent(SDL_Renderer *renderer)
{
    if (captured_frame)
    {
        int width, height;
        SDL_RenderGetLogicalSize(renderer, &width, &height);
        assert(SDL_RenderSetLogicalSize(renderer, 0, 0) == 0);
        assert(SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32,
                                    captured_frame->pixels, captured_frame->pitch) == 0);
        assert(SDL_RenderSetLogicalSize(renderer, width, height) == 0);
    }
    if (displays.count >= 0)
    {
        auto *window = SDL_RenderGetWindow(renderer);
        if (window == displays.primary)
            ++displays.primary_frames;
        else if (window == displays.secondary)
            ++displays.secondary_frames;
        if (displays.count >= 2 && displays.failure == DisplayFailure::none)
        {
            int width, height;
            assert(SDL_GetRendererOutputSize(renderer, &width, &height) == 0);
            SDL_Rect center{width / 2, height / 2, 1, 1};
            Uint8 rgba[4];
            assert(SDL_RenderReadPixels(renderer, &center, SDL_PIXELFORMAT_RGBA32, rgba, sizeof(rgba)) == 0);
            const bool empty = rgba[0] == 0 && rgba[1] == 0 && rgba[2] == 0;
            // The menu center is empty above; console artwork fills it below.
            assert(empty == ((window == displays.primary) != displays.swap_screens));
        }
    }
    __real_SDL_RenderPresent(renderer);
}

// Model handheld ALSA hardware that rejects mono while retaining real SDL audio.
extern "C" SDL_AudioDeviceID __wrap_SDL_OpenAudioDevice(
    const char *device, int capture, const SDL_AudioSpec *desired,
    SDL_AudioSpec *obtained, int allowed_changes)
{
    if (desired->channels != 2)
    {
        SDL_SetError("Test audio device requires stereo");
        return 0;
    }
    assert(allowed_changes & SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
    assert(allowed_changes & SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    return __real_SDL_OpenAudioDevice(device, capture, desired, obtained, allowed_changes);
}

extern "C" SDL_AudioStream *__wrap_SDL_NewAudioStream(
    SDL_AudioFormat source_format, Uint8 source_channels, int source_rate,
    SDL_AudioFormat output_format, Uint8 output_channels, int output_rate)
{
    assert(source_channels == 1 && output_channels == 2);
    return __real_SDL_NewAudioStream(source_format, source_channels, source_rate,
                                    output_format, output_channels, output_rate);
}

static void check_controller()
{
#if SDL_VERSION_ATLEAST(2, 0, 14)
    // Virtual devices let SDL exercise the same polling/hotplug path as a handheld.
    const int index = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,
                                               SDL_CONTROLLER_AXIS_MAX, SDL_CONTROLLER_BUTTON_MAX, 0);
    assert(index >= 0);
    SDL_Joystick *joystick = SDL_JoystickOpen(index);
    assert(joystick);
    Controller controller;
    SDL_Event added{};
    added.type = SDL_CONTROLLERDEVICEADDED;
    controller.handle_event(added);
    assert(SDL_JoystickSetVirtualButton(joystick, SDL_CONTROLLER_BUTTON_A, 1) == 0);
    SDL_JoystickUpdate();
    auto state = controller.poll();
    assert(state.held == CTL_A && state.pressed == CTL_A);
    state = controller.poll();
    assert(state.held == CTL_A && state.pressed == 0);
    assert(SDL_JoystickSetVirtualButton(joystick, SDL_CONTROLLER_BUTTON_A, 0) == 0);
    assert(SDL_JoystickSetVirtualAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, -16000) == 0);
    SDL_JoystickUpdate();
    state = controller.poll();
    assert(state.held == 0 && state.pressed == 0);
    assert(SDL_JoystickSetVirtualAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, -16001) == 0);
    SDL_JoystickUpdate();
    state = controller.poll();
    assert(state.held == CTL_DPAD_LEFT && state.pressed == CTL_DPAD_LEFT);
    assert(SDL_JoystickSetVirtualAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, 16000) == 0);
    assert(SDL_JoystickSetVirtualAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, 16000) == 0);
    SDL_JoystickUpdate();
    state = controller.poll();
    assert(state.held == 0 && state.pressed == 0);
    assert(SDL_JoystickSetVirtualAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, 16001) == 0);
    assert(SDL_JoystickSetVirtualAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, 16001) == 0);
    SDL_JoystickUpdate();
    state = controller.poll();
    assert(state.held == (CTL_DPAD_RIGHT | CTL_DPAD_DOWN));
    assert(state.pressed == state.held);
    assert(SDL_JoystickSetVirtualAxis(joystick, SDL_CONTROLLER_AXIS_LEFTX, 0) == 0);
    assert(SDL_JoystickSetVirtualAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, -16000) == 0);
    SDL_JoystickUpdate();
    assert(controller.poll().held == 0);
    assert(SDL_JoystickSetVirtualAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, -16001) == 0);
    SDL_JoystickUpdate();
    state = controller.poll();
    assert(state.held == CTL_DPAD_UP && state.pressed == CTL_DPAD_UP);
    assert(SDL_JoystickSetVirtualAxis(joystick, SDL_CONTROLLER_AXIS_LEFTY, 0) == 0);
    SDL_JoystickUpdate();
    assert(controller.poll().held == 0);

    struct Binding
    {
        SDL_GameControllerButton button;
        uint32_t bit;
    };
    constexpr Binding bindings[] = {
        {SDL_CONTROLLER_BUTTON_DPAD_UP, CTL_DPAD_UP},
        {SDL_CONTROLLER_BUTTON_DPAD_DOWN, CTL_DPAD_DOWN},
        {SDL_CONTROLLER_BUTTON_DPAD_LEFT, CTL_DPAD_LEFT},
        {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, CTL_DPAD_RIGHT},
        {SDL_CONTROLLER_BUTTON_A, CTL_A},
        {SDL_CONTROLLER_BUTTON_B, CTL_B},
        {SDL_CONTROLLER_BUTTON_X, CTL_X},
        {SDL_CONTROLLER_BUTTON_Y, CTL_Y},
        {SDL_CONTROLLER_BUTTON_START, CTL_START},
        {SDL_CONTROLLER_BUTTON_BACK, CTL_SELECT},
        {SDL_CONTROLLER_BUTTON_LEFTSHOULDER, CTL_L1},
        {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, CTL_MENU},
        {SDL_CONTROLLER_BUTTON_GUIDE, CTL_MENU},
    };
    for (const auto &binding : bindings)
    {
        assert(SDL_JoystickSetVirtualButton(joystick, binding.button, 1) == 0);
        SDL_JoystickUpdate();
        state = controller.poll();
        assert(state.held == binding.bit && state.pressed == binding.bit);
        assert(controller.poll().pressed == 0);
        assert(SDL_JoystickSetVirtualButton(joystick, binding.button, 0) == 0);
        SDL_JoystickUpdate();
        state = controller.poll();
        assert(state.held == 0 && state.pressed == 0);
    }
    assert(SDL_JoystickSetVirtualButton(joystick, SDL_CONTROLLER_BUTTON_GUIDE, 1) == 0);
    SDL_JoystickUpdate();
    assert(controller.poll().pressed == CTL_MENU);
    assert(SDL_JoystickSetVirtualButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, 1) == 0);
    SDL_JoystickUpdate();
    assert(controller.poll().pressed == 0);
    assert(SDL_JoystickSetVirtualButton(joystick, SDL_CONTROLLER_BUTTON_GUIDE, 0) == 0);
    SDL_JoystickUpdate();
    assert(controller.poll().held == CTL_MENU);
    assert(SDL_JoystickSetVirtualButton(joystick, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, 0) == 0);
    SDL_JoystickUpdate();
    assert(controller.poll().held == 0);
    assert(SDL_JoystickDetachVirtual(index) == 0);
    SDL_Event removed{};
    removed.type = SDL_CONTROLLERDEVICEREMOVED;
    controller.handle_event(removed);
    state = controller.poll();
    assert(state.held == 0 && state.pressed == 0);
    SDL_JoystickClose(joystick);
#endif
}

static std::array<Uint8, 3> pixel(SDL_Surface *surface, int x, int y)
{
    const auto *row = static_cast<const Uint8 *>(surface->pixels) + y * surface->pitch;
    Uint32 value;
    std::memcpy(&value, row + x * sizeof(value), sizeof(value));
    Uint8 r, g, b, a;
    SDL_GetRGBA(value, surface->format, &r, &g, &b, &a);
    return {r, g, b};
}

static void check_panels(SDL_Renderer *renderer, SDL_Surface *surface)
{
    auto *bottom_surface = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_RGBA32);
    assert(bottom_surface);
    auto *bottom_renderer = SDL_CreateSoftwareRenderer(bottom_surface);
    assert(bottom_renderer);
    {
        const auto *game = load_game(0);
        assert(game);
        std::vector<Segment> segments(game->segment, game->segment + game->size_segment);
        const bool split = segments.size() == 2 && !(game->segment_info[3] & 2);
        GameRenderer top(renderer, Panel::top);
        GameRenderer bottom(bottom_renderer, Panel::bottom);
        GameRenderer combined(renderer);
        top.load(*game);
        bottom.load(*game);
        combined.load(*game);
        top.draw(*game, segments, true);
        bottom.draw(*game, segments, true);
        const std::array<Uint8, 3> empty{0, 0, 0};
        const uint32_t fill = (game->segment_info[3] & 1) ? 0x080908 : g_settings.background_color;
        const std::array<Uint8, 3> lcd{Uint8(fill >> 16), Uint8(fill >> 8), Uint8(fill)};
        assert(pixel(surface, 320, 240) == empty);
        assert(pixel(bottom_surface, 320, 240) != empty);
        const size_t bytes = bottom_surface->pitch * bottom_surface->h;
        std::vector<Uint8> console(bytes);
        std::memcpy(console.data(), bottom_surface->pixels, bytes);
        top.draw(*game, segments, false);
        bottom.draw(*game, segments, false);
        if (!split)
        {
            assert(std::memcmp(console.data(), bottom_surface->pixels, bytes) == 0);
            std::vector<Uint8> top_game(surface->pitch * surface->h);
            std::memcpy(top_game.data(), surface->pixels, top_game.size());
            UiState top_only;
            top_only.mode = AppMode::game;
            top_only.top_only = true;
            combined.draw(*game, segments, top_only);
            assert(std::memcmp(top_game.data(), surface->pixels, top_game.size()) == 0);
        }

        std::array<uint16_t, 8> info{};
        std::copy_n(game->segment_info, 6, info.begin());
        if (segments.size() == 2)
        {
            info[6] = 120;
            info[7] = 60;
        }
        for (auto &segment : segments)
        {
            segment.buffer_state = true;
            segment.pos_scr[0] = segment.screen == 0 ? 24 : 48;
            segment.pos_scr[1] = 24;
        }
        GW_rom isolated(game->name, game->ref, game->date, game->rom, game->size_rom,
                        game->melody, game->size_melody, game->path_segment,
                        segments.data(), segments.size(), info.data(), "", nullptr, "", nullptr);
        top.load(isolated);
        bottom.load(isolated);
        top.draw(isolated, segments, false);
        bottom.draw(isolated, segments, false);
        if (split)
        {
            // Distinct segments and aspect ratios prove each panel draws only its LCD.
            assert(pixel(surface, 200, 200) != lcd);
            assert(pixel(bottom_surface, 200, 200) == lcd);
            assert(pixel(surface, 280, 220) == lcd);
            assert(pixel(bottom_surface, 280, 220) != lcd);
            assert(pixel(surface, 20, 20) == lcd);
            assert(pixel(bottom_surface, 20, 20) == empty);
        }
        else
            assert(pixel(bottom_surface, 320, 240) == empty);
        bottom.draw(isolated, segments, true);
        assert(pixel(bottom_surface, 320, 240) == empty);
        {
            App app(renderer, bottom_renderer);
            app.select(0);
            app.draw();
            assert(std::memcmp(console.data(), bottom_surface->pixels, bytes) == 0);
            app.start(false);
            app.tick(0);
            app.draw();
            if (!split)
                assert(std::memcmp(console.data(), bottom_surface->pixels, bytes) == 0);
            else
                assert(std::memcmp(console.data(), bottom_surface->pixels, bytes) != 0);
            app.return_to_menu();
            app.draw();
            assert(std::memcmp(console.data(), bottom_surface->pixels, bytes) == 0);
            app.select(0);
            app.draw();
            assert(std::memcmp(console.data(), bottom_surface->pixels, bytes) == 0);
        }
    }
    SDL_DestroyRenderer(bottom_renderer);
    SDL_FreeSurface(bottom_surface);
}

static void check_displays(const char *pack, const char *storage)
{
    const std::pair<int, DisplayFailure> cases[] = {
        {0, DisplayFailure::none}, {1, DisplayFailure::none}, {2, DisplayFailure::none},
        {3, DisplayFailure::none}, {2, DisplayFailure::window}, {2, DisplayFailure::placement},
        {2, DisplayFailure::renderer}, {2, DisplayFailure::logical_size},
    };
    for (const auto &[count, failure] : cases)
    {
        UiState preferences;
        load_display_preferences(preferences);
        displays = {count, failure, preferences.swap_screens};
        assert(SDL_Init(SDL_INIT_EVENTS) == 0);
        SDL_Event quit{};
        quit.type = SDL_QUIT;
        assert(SDL_PushEvent(&quit) == 1);
        if (count == 0)
        {
            bool rejected = false;
            try { run_application(pack, storage); }
            catch (const std::runtime_error &error)
            {
                rejected = std::string(error.what()).find("SDL video displays") != std::string::npos;
            }
            assert(rejected && displays.window_attempts == 0);
            SDL_Quit();
            gw_pack::unload();
            continue;
        }
        run_application(pack, storage);
        const bool dual = count >= 2 && failure == DisplayFailure::none;
        assert(displays.window_attempts == (count >= 2 ? 2 : 1));
        assert(displays.primary_frames == 1);
        assert(displays.secondary_frames == (dual ? 1 : 0));
        assert(SDL_WasInit(0) == 0);
    }
    displays = {};
}

static void press(App &app, uint32_t bit)
{
    assert(!app.handle_input({bit, bit}));
}

static void focus_setting(App &app, Setting setting, bool dual)
{
    const auto items = settings_items(app.ui(), dual);
    assert(app.ui().settings_open);
    assert(std::find(items.begin(), items.end(), setting) != items.end());
    while (items[app.ui().settings_row] != setting)
        press(app, CTL_DPAD_DOWN);
}

static std::vector<unsigned char> cpu_snapshot(SM5XX *cpu)
{
    FILE *file = std::tmpfile();
    assert(file && cpu->save_state(file));
    assert(std::fseek(file, 0, SEEK_END) == 0);
    const auto size = std::ftell(file);
    assert(size > 0 && std::fseek(file, 0, SEEK_SET) == 0);
    std::vector<unsigned char> data(size_t(size), 0);
    assert(std::fread(data.data(), 1, data.size(), file) == data.size());
    assert(std::fclose(file) == 0);
    return data;
}

static void check_clock_startup(SDL_Renderer *renderer)
{
    std::tm clock{};
    clock.tm_year = 126;
    clock.tm_mon = 9;
    clock.tm_mday = 31;
    clock.tm_hour = 15;
    clock.tm_min = 47;
    clock.tm_sec = 36;
    test_time = std::mktime(&clock);
    assert(test_time > 0);
    {
        App app(renderer);
        app.select(0);
        app.start_fresh();
        assert(app.cpu()->get_cpu_type_id() == 1);
        constexpr size_t ram_offset = 3 * sizeof(ProgramCounter) + sizeof(RAMAddress) +
                                      sizeof(uint8_t) + sizeof(bool);
        auto state = cpu_snapshot(app.cpu());
        assert(state.at(ram_offset + 4 * 16 + 12) == 3);
        app.cpu()->time_set(false);
        app.cpu()->set_time(1, 2, 3);
        app.cpu()->time_set(true);
        app.return_to_menu();
        app.start(true);
        state = cpu_snapshot(app.cpu());
        assert(state.at(ram_offset + 4 * 16 + 4) == 2);
        assert(state.at(ram_offset + 4 * 16 + 3) == 3);
        assert(state.at(ram_offset + 4 * 16 + 2) == 4);
        assert(state.at(ram_offset + 4 * 16 + 1) == 7);
        assert(state.at(ram_offset + 4 * 16 + 6) == 3);
        assert(state.at(ram_offset + 4 * 16 + 5) == 6);
    }
    test_time = 0;
}

static void check_settings(SDL_Renderer *renderer, SDL_Surface *surface, bool dual)
{
    auto *second_surface = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_RGBA32);
    assert(second_surface);
    auto *second_renderer = SDL_CreateSoftwareRenderer(second_surface);
    assert(second_renderer);
    {
        reset_settings_to_default();
        UiState defaults;
        save_display_preferences(defaults);
        App app(renderer, dual ? second_renderer : nullptr);
        app.initialize();
        app.select(0);
        press(app, CTL_MENU);
        const auto menu_items = settings_items(app.ui(), dual);
        assert(menu_items == (dual ?
            std::vector<Setting>{Setting::segments, Setting::background, Setting::swap, Setting::exit_app, Setting::close} :
            std::vector<Setting>{Setting::segments, Setting::background, Setting::exit_app, Setting::close}));
        assert(std::find(menu_items.begin(), menu_items.end(), Setting::quit_game) == menu_items.end());
        assert(std::find(menu_items.begin(), menu_items.end(), Setting::top_only) == menu_items.end());
        assert((std::find(menu_items.begin(), menu_items.end(), Setting::swap) != menu_items.end()) == dual);
        focus_setting(app, Setting::segments, dual);
        press(app, CTL_DPAD_RIGHT);
        assert(g_settings.segment_marking_alpha == 10);
        press(app, CTL_DPAD_RIGHT);
        assert(g_settings.segment_marking_alpha == 0);
        press(app, CTL_DPAD_RIGHT);
        assert(g_settings.segment_marking_alpha == 5);
        focus_setting(app, Setting::background, dual);
        for (int i = 1; i <= NUM_BG_PRESETS; ++i)
        {
            press(app, CTL_DPAD_RIGHT);
            assert(g_settings.background_color == BACKGROUND_PRESETS[i % NUM_BG_PRESETS].color);
        }
        if (dual)
        {
            focus_setting(app, Setting::swap, true);
            press(app, CTL_A);
            assert(app.ui().swap_screens);
        }
        press(app, CTL_B);
        assert(!app.ui().settings_open && app.in_menu());
        if (dual)
        {
            app.draw();
            const std::array<Uint8, 3> black{0, 0, 0};
            assert(pixel(surface, 320, 240) != black);
            assert(pixel(second_surface, 320, 240) == black);
        }
        {
            App restored(renderer, dual ? second_renderer : nullptr);
            restored.initialize();
            assert(restored.ui().swap_screens == dual);
            assert(!restored.ui().top_only);
            assert(g_settings.segment_marking_alpha == 5);
            assert(g_settings.background_color == BACKGROUND_PRESETS[0].color);
        }
        const PointerId finger{1, 1};
        const bool touch_bottom = !dual || !app.ui().swap_screens;
        app.handle_pointer(finger, 320, 370, PointerPhase::press, touch_bottom);
        assert(app.ui().mode == AppMode::load_prompt);
        app.handle_pointer(finger, 320, 370, PointerPhase::release, touch_bottom);
        app.handle_pointer(finger, 550, 370, PointerPhase::press, touch_bottom);
        assert(app.ui().mode == AppMode::game);
        app.handle_pointer(finger, 550, 370, PointerPhase::release, touch_bottom);
        app.tick(0);
        const auto before = cpu_snapshot(app.cpu());
        press(app, CTL_MENU);
        assert(app.ui().settings_open);
        assert(settings_items(app.ui(), dual) == (dual ?
            std::vector<Setting>{Setting::segments, Setting::background, Setting::swap, Setting::quit_game, Setting::close} :
            std::vector<Setting>{Setting::segments, Setting::background, Setting::quit_game, Setting::top_only, Setting::close}));
        for (int i = 0; i < 120; ++i) app.tick(CTL_START | CTL_A);
        assert(cpu_snapshot(app.cpu()) == before);
        press(app, CTL_MENU);
        app.tick(0);
        assert(cpu_snapshot(app.cpu()) != before);
        press(app, CTL_MENU);
        if (!dual)
        {
            focus_setting(app, Setting::top_only, false);
            press(app, CTL_A);
            assert(app.ui().top_only && !app.ui().settings_open);
            app.draw();
            press(app, CTL_MENU);
        }
        app.draw();
        focus_setting(app, Setting::quit_game, dual);
        app.draw();
        assert(SDL_SaveBMP(dual ? second_surface : surface,
            storage_path(dual ? "settings-dual.bmp" : "settings-single.bmp").c_str()) == 0);
        press(app, CTL_A);
        assert(app.in_menu() && !app.playing() && !app.cpu() && !app.ui().settings_open && !app.ui().top_only);
        assert(save_state_exists(app.selected_index()));
        reset_settings_to_default();
        assert(save_settings());
        save_display_preferences(defaults);
    }
    SDL_DestroyRenderer(second_renderer);
    SDL_FreeSurface(second_surface);
}

static void check_square_output()
{
    for (const int height : {720, 721})
    {
        auto *surface = SDL_CreateRGBSurfaceWithFormat(0, 720, height, 32, SDL_PIXELFORMAT_RGBA32);
        assert(surface);
        auto *renderer = SDL_CreateSoftwareRenderer(surface);
        assert(renderer && SDL_RenderSetLogicalSize(renderer, 640, 480) == 0);
        {
            const auto *game = load_game(0);
            assert(game);
            std::vector<Segment> segments(game->segment, game->segment + game->size_segment);
            GameRenderer graphics(renderer);
            graphics.load(*game);
            UiState ui;
            graphics.draw(*game, segments, ui);
            const std::array<Uint8, 3> black{0, 0, 0};
            assert(pixel(surface, 360, height - 1) != black);
            int artwork_width = 0;
            for (int x = 0; x < 720; ++x)
                artwork_width += pixel(surface, x, height - 1) != black;
            assert(std::abs(artwork_width - (height - height / 2)) <= 2);
            assert(pixel(surface, 175, height - 1) == black);
            const auto undimmed = pixel(surface, 360, height - 1);
            graphics.draw(*game, segments, ui, true);
            const auto dimmed = pixel(surface, 360, height - 1);
            assert(dimmed[2] < undimmed[2]);
            int width, logical_h;
            SDL_RenderGetLogicalSize(renderer, &width, &logical_h);
            assert(width == 640 && logical_h == 480);
            ui.mode = AppMode::game;
            ui.top_only = true;
            graphics.draw(*game, segments, ui);
            assert(pixel(surface, 0, height / 2) != black);
            assert(pixel(surface, 360, 0) == black);
        }
        SDL_DestroyRenderer(renderer);
        SDL_FreeSurface(surface);
    }
}

static void check_exit_app(SDL_Renderer *renderer)
{
    auto *surface = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_RGBA32);
    assert(surface);
    auto *secondary = SDL_CreateSoftwareRenderer(surface);
    assert(secondary);
    for (const bool dual : {false, true})
        for (const bool prompt : {false, true})
            for (const bool pointer : {false, true})
            {
                App app(renderer, dual ? secondary : nullptr);
                app.select(0);
                if (prompt)
                    press(app, CTL_A);
                app.toggle_settings();
                focus_setting(app, Setting::exit_app, dual);
                assert(!app.handle_input({CTL_DPAD_RIGHT, CTL_DPAD_RIGHT}));
                const auto mode = app.ui().mode;
                if (pointer)
                {
                    const PointerId finger{7, 1};
                    app.handle_pointer(finger, 320, 78 + app.ui().settings_row * 50 + 23,
                                       PointerPhase::press, true);
                    app.handle_pointer(finger, 320, 79, PointerPhase::press, true);
                    assert(app.handle_input({}));
                }
                else
                    assert(app.handle_input({CTL_A, CTL_A}));
                assert(app.handle_input({CTL_A, CTL_A}));
                assert(app.ui().mode == mode && !app.cpu() && !app.playing());
                app.save();
            }
    SDL_DestroyRenderer(secondary);
    SDL_FreeSurface(surface);
}

static void check_resized_output()
{
    auto *window = SDL_CreateWindow("Layout regression", 0, 0, 640, 480,
                                   SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    assert(window);
    auto *renderer = SDL_CreateRenderer(window, -1, 0);
    assert(renderer);
    {
        const auto *game = load_game(0);
        assert(game);
        std::vector<Segment> segments(game->segment, game->segment + game->size_segment);
        GameRenderer graphics(renderer);
        graphics.load(*game);
        for (const auto size : {SDL_Point{640, 480}, SDL_Point{720, 720}, SDL_Point{960, 480},
                                SDL_Point{720, 721}})
        {
            SDL_SetWindowSize(window, size.x, size.y);
            SDL_PumpEvents();
            assert(SDL_RenderSetLogicalSize(renderer, 640, 480) == 0);
            int width, height;
            assert(SDL_GetRendererOutputSize(renderer, &width, &height) == 0);
            assert(width == size.x && height == size.y);
            auto *surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
            assert(surface);
            captured_frame = surface;
            graphics.draw(*game, segments, true);
            captured_frame = nullptr;
            SDL_RenderGetLogicalSize(renderer, &width, &height);
            assert(width == 640 && height == 480);
            const std::array<Uint8, 3> black{0, 0, 0};
            assert(pixel(surface, size.x / 2, size.y - 1) != black);
            SDL_FreeSurface(surface);
            App app(renderer);
            app.select(0);
            app.toggle_settings();
            focus_setting(app, Setting::exit_app, false);
            app.draw();
            int x, y;
            SDL_RenderLogicalToWindow(renderer, 320, 78 + app.ui().settings_row * 50 + 23, &x, &y);
            float logical_x, logical_y;
            SDL_RenderWindowToLogical(renderer, x, y, &logical_x, &logical_y);
            app.handle_pointer({8, 1}, logical_x, logical_y, PointerPhase::press, true);
            assert(app.handle_input({}));
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

static void check_font(SDL_Renderer *renderer, SDL_Surface *surface)
{
    assert(text_width("WWWW") > text_width("IIII"));
    assert(text_width("caf\xc3\xa9") > 0);
    for (const char *valid : {"\xc2\x80", "\xdf\xbf", "\xe0\xa0\x80", "\xed\x9f\xbf",
                             "\xee\x80\x80", "\xef\xbf\xbf", "\xf0\x90\x80\x80",
                             "\xf4\x8f\xbf\xbf"})
        assert(text_width(valid) >= 0);
    for (const char *invalid : {"\x80", "\xc0\xaf", "\xc2", "\xc2\x41", "\xe0\x80\x80",
                               "\xed\xa0\x80", "\xf0\x80\x80\x80", "\xf4\x90\x80\x80",
                               "\xf5\x80\x80\x80", "\xff"})
    {
        bool rejected = false;
        try { text_width(invalid); }
        catch (const std::runtime_error &) { rejected = true; }
        assert(rejected);
    }
    assert(std::strstr(font_license(), "SIL Open Font License, Version 1.1"));
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    Font font(renderer);
    font.draw(40, 40, "Audiowide caf\xc3\xa9", 4);
    SDL_RenderPresent(renderer);
    int antialiased = 0, solid = 0;
    for (int y = 30; y < 110; ++y)
        for (int x = 30; x < 620; ++x)
        {
            const auto color = pixel(surface, x, y);
            assert(color[0] == color[1] && color[1] == color[2]);
            antialiased += color[0] > 0 && color[0] < 255;
            solid += color[0] == 255;
        }
    assert(antialiased > 100 && solid > 100);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    font.draw(40, 40, "Audiowide caf\xc3\xa9", 4);
    SDL_RenderPresent(renderer);
    int repeated = 0;
    for (int y = 30; y < 110; ++y)
        for (int x = 30; x < 620; ++x)
            repeated += pixel(surface, x, y)[0] > 0;
    assert(repeated == antialiased + solid);
    assert(iconv_calls == 0);
}

static void check_visual_settings(SDL_Renderer *renderer, SDL_Surface *surface)
{
    const auto *game = load_game(0);
    assert(game);
    std::vector<Segment> segments(game->segment, game->segment + game->size_segment);
    for (auto &segment : segments)
    {
        segment.pos_scr[0] = 24;
        segment.pos_scr[1] = 24;
        segment.buffer_state = false;
    }
    GW_rom isolated(game->name, game->ref, game->date, game->rom, game->size_rom,
                    game->melody, game->size_melody, game->path_segment,
                    segments.data(), segments.size(), game->segment_info, "", nullptr, "", nullptr);
    GameRenderer top(renderer, Panel::top);
    top.load(isolated);
    g_settings.background_color = 0xffffff;
    g_settings.segment_marking_alpha = 0;
    top.draw(isolated, segments, false);
    if (!(game->segment_info[3] & 1) && !(game->segment_info[3] & 2))
    {
        const auto none = pixel(surface, 200, 200);
        const std::array<Uint8, 3> white{255, 255, 255};
        assert(none == white);
        g_settings.segment_marking_alpha = 5;
        top.draw(isolated, segments, false);
        const auto light = pixel(surface, 200, 200);
        g_settings.segment_marking_alpha = 10;
        top.draw(isolated, segments, false);
        const auto dark = pixel(surface, 200, 200);
        assert(none[0] > light[0] && light[0] > dark[0]);
        segments[0].buffer_state = true;
        top.draw(isolated, segments, false);
        assert(pixel(surface, 200, 200)[0] == 30);
    }
    top.draw(isolated, segments, true);
    int header = 0, title = 0, year = 0;
    for (int y = 40; y < 210; ++y)
        for (int x = 0; x < 640; ++x)
        {
            const auto color = pixel(surface, x, y);
            header += y < 80 && color == std::array<Uint8, 3>{136, 136, 136};
            title += y >= 90 && y < 150 && color == std::array<Uint8, 3>{255, 255, 255};
            year += y >= 160 && color == std::array<Uint8, 3>{204, 204, 204};
        }
    assert(header > 10 && title > 10 && year > 10);
    reset_settings_to_default();
}

static uint8_t index_of(const char *ref)
{
    for (size_t i = 0; i < gw_pack::game_count(); ++i)
        if (get_ref(uint8_t(i)) == ref)
            return uint8_t(i);
    assert(false);
    return 0;
}

static void check_navigation(SDL_Renderer *renderer)
{
    if (gw_pack::game_count() == 1)
    {
        App app(renderer);
        app.select(0);
        press(app, CTL_DPAD_UP);
        press(app, CTL_DPAD_DOWN);
        assert(app.selected_index() == 0);
        return;
    }
    if (gw_pack::game_count() != 6)
        return;
    App app(renderer);
    app.select(index_of("AC_01"));
    press(app, CTL_DPAD_RIGHT);
    assert(get_ref(app.selected_index()) == "JR_55");
    press(app, CTL_DPAD_RIGHT);
    assert(get_ref(app.selected_index()) == "AC_01");
    press(app, CTL_DPAD_LEFT);
    assert(get_ref(app.selected_index()) == "JR_55");
    press(app, CTL_DPAD_UP);
    assert(load_game(app.selected_index())->manufacturer == GW_rom::MANUFACTURER_TRONICA);
    app.select(index_of("TM_02"));
    press(app, CTL_DPAD_DOWN);
    assert(get_ref(app.selected_index()) == "JR_55");
    press(app, CTL_DPAD_UP);
    assert(get_ref(app.selected_index()) == "TM_02");
    press(app, CTL_DPAD_UP);
    assert(load_game(app.selected_index())->manufacturer == GW_rom::MANUFACTURER_ELEKTRONIKA);
    app.select(index_of("EL_02"));
    {
        App restored(renderer);
        restored.initialize();
        assert(get_ref(restored.selected_index()) == "EL_02");
        press(restored, CTL_DPAD_DOWN);
        assert(get_ref(restored.selected_index()) == "TM_02");
    }
    app.select(index_of("EL_02"));
}

static void check_persistence_errors()
{
    const auto root = storage_root();
    const auto settings_path = storage_path("yokoi_gw_settings.dat");
    FILE *file = std::fopen(settings_path.c_str(), "wb");
    assert(file && std::fputc(0, file) != EOF && std::fclose(file) == 0);
    g_settings.background_color = 0;
    load_settings();
    assert(g_settings.background_color == BACKGROUND_PRESETS[0].color);
    assert(save_settings());
    const auto preferences_path = storage_path("yokoi_linux_preferences.dat");
    file = std::fopen(preferences_path.c_str(), "wb");
    assert(file && std::fputc(0, file) != EOF && std::fclose(file) == 0);
    UiState preferences;
    preferences.swap_screens = true;
    load_display_preferences(preferences);
    assert(!preferences.swap_screens);
    save_display_preferences(preferences);
    for (const unsigned char version : {'1', '2'})
    {
        file = std::fopen(preferences_path.c_str(), "wb");
        const unsigned char retired[] = {'Y', 'L', 'P', version, 1, 1};
        assert(file && std::fwrite(retired, 1, sizeof(retired), file) == sizeof(retired) &&
               std::fclose(file) == 0);
        preferences.swap_screens = false;
        load_display_preferences(preferences);
        assert(preferences.swap_screens);
    }
    save_display_preferences(preferences);
    file = std::fopen(preferences_path.c_str(), "rb");
    std::array<unsigned char, 6> saved{};
    assert(file && std::fread(saved.data(), 1, saved.size(), file) == saved.size() &&
           std::fgetc(file) == EOF && std::fclose(file) == 0);
    const std::array<unsigned char, 6> expected{'Y', 'L', 'P', '1', 1, 0};
    assert(saved == expected);
    load_display_preferences(preferences);
    assert(preferences.swap_screens);
    set_storage_root(root + "/nonexistent-parent");
    assert(!save_settings());
    assert(!save_last_selected_game(0, "AC_01"));
    bool rejected = false;
    try { save_display_preferences(preferences); }
    catch (const std::runtime_error &) { rejected = true; }
    assert(rejected);
    set_storage_root(root);
}

int main(int argc, char **argv)
{
    assert(argc == 3 || argc == 4);
    std::string error;
    assert(gw_pack::load(argv[1], &error));
    set_storage_root(argv[2]);
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) == 0);
    auto *surface = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_RGBA32);
    auto *renderer = SDL_CreateSoftwareRenderer(surface);
    assert(renderer);
    if (load_game(0)->name == "Clock startup")
    {
        check_clock_startup(renderer);
        SDL_DestroyRenderer(renderer);
        SDL_FreeSurface(surface);
        SDL_Quit();
        gw_pack::unload();
        std::puts("PASS: startup clock latch and current-time save restoration");
        return 0;
    }
    if (argc == 4)
    {
        App restored(renderer);
        restored.initialize();
        assert(get_ref(restored.selected_index()) == argv[3]);
    }
    else if (gw_pack::game_count() == 6 &&
             !std::filesystem::exists(storage_path("yokoi_gw_last_game_by_mfr.dat")))
    {
        App initial(renderer);
        initial.initialize();
        assert(get_ref(initial.selected_index()) == "JR_55");
    }
    {
        auto texture = load_texture(renderer, "gfx/test.png");
        SDL_FRect destination{0, 0, 3, 2};
        copy_texture(renderer, texture.get(), SDL_Rect{2, 1, 3, 2}, destination);
        SDL_RenderPresent(renderer);
        Uint8 r, g, b, a;
        SDL_GetRGBA(*static_cast<Uint32 *>(surface->pixels), surface->format, &r, &g, &b, &a);
        assert(r == 128 && g == 160 && b == 50 && a == 255);
    }
    {
        App app(renderer);
        app.select(0);
        app.draw();
        SDL_SaveBMP(surface, (std::string(argv[2]) + "/menu.bmp").c_str());
        assert(app.open_audio());
        app.start(false);
        assert(app.audio_ready());
        for (int i = 0; i < 120; ++i)
            app.tick(i < 10 ? CTL_START : CTL_DPAD_LEFT);
        app.draw();
        app.save();
        assert(save_state_exists(0));
        assert(load_game_state(app.cpu(), 0));
        SDL_SaveBMP(surface, (std::string(argv[2]) + "/game.bmp").c_str());
        app.return_to_menu();
        app.draw();
        assert(app.cpu() == nullptr && !app.playing());
        assert(!app.handle_input({CTL_B, CTL_B}));
        assert(app.in_menu());
        assert(!app.handle_input({CTL_A, CTL_A}));
        assert(app.ui().mode == AppMode::load_prompt && app.ui().load_save);
        app.draw();
        assert(SDL_SaveBMP(surface, storage_path("prompt.bmp").c_str()) == 0);
        int prompt_header = 0;
        for (int y = 20; y < 100; ++y)
            for (int x = 160; x < 480; ++x)
                prompt_header += pixel(surface, x, y)[0] > 0;
        assert(prompt_header > 100);
        assert(!app.handle_input({CTL_DPAD_LEFT, CTL_DPAD_LEFT}));
        assert(!app.ui().load_save);
        assert(!app.handle_input({CTL_A, CTL_A}));
        assert(app.cpu() && !app.in_menu());
        assert(!app.handle_input({CTL_L1 | CTL_B, CTL_B}));
        assert(app.in_menu() && !app.playing());
        assert(!app.handle_input({CTL_B, CTL_B}));
        assert(app.in_menu());
        assert(!app.handle_input({CTL_A, CTL_A}));
        assert(app.ui().mode == AppMode::load_prompt);
        assert(!app.handle_input({CTL_A, CTL_A}));
        assert(!app.in_menu() && app.audio_ready());
        app.return_to_menu();
        app.start_fresh();
        assert(!app.in_menu() && app.cpu() && app.audio_ready());
        assert(app.handle_input({CTL_START | CTL_SELECT, 0}));
        app.return_to_menu();
        app.select(0);
        app.start(true);
        app.tick(0);
    }
    check_panels(renderer, surface);
    check_settings(renderer, surface, false);
    check_settings(renderer, surface, true);
    check_square_output();
    check_exit_app(renderer);
    check_resized_output();
    check_visual_settings(renderer, surface);
    check_font(renderer, surface);
    check_persistence_errors();
    check_navigation(renderer);
    check_controller();
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    SDL_Quit();
    gw_pack::unload();
    // Queue an exit before startup so the real window/event loop runs one frame.
    assert(SDL_Init(SDL_INIT_EVENTS) == 0);
    SDL_Event quit{};
    quit.type = SDL_QUIT;
    assert(SDL_PushEvent(&quit) == 1);
    run_application(argv[1], argv[2]);
    assert(SDL_WasInit(0) == 0);
    check_displays(argv[1], argv[2]);
    std::puts("PASS: pack, CPU, audio, controller, Android menu/settings/persistence/touch, dual panels/fallback");
}
