#include <SDL.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include "core/yokoi_app.h"
#include "core/yokoi_runtime.h"
#include "input/yokoi_controller.h"
#include "render/yokoi_graphics.h"
#include "std/gw_pack.h"
#include "std/platform_paths.h"
#include "std/savestate.h"
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

enum class DisplayFailure { none, window, placement, renderer, logical_size };
static struct
{
    int count = -1;
    DisplayFailure failure = DisplayFailure::none;
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
            const bool empty = rgba[0] == 22 && rgba[1] == 27 && rgba[2] == 33;
            // The menu center is empty above; console artwork fills it below.
            assert(empty == (window == displays.primary));
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
        const std::array<Uint8, 3> empty{22, 27, 33};
        const std::array<Uint8, 3> lcd{195, 200, 178};
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
            combined.draw(*game, segments, false);
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
            assert(pixel(surface, 260, 210) == lcd);
            assert(pixel(bottom_surface, 260, 210) != lcd);
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
        displays = {count, failure};
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

int main(int argc, char **argv)
{
    assert(argc == 3);
    std::string error;
    assert(gw_pack::load(argv[1], &error));
    set_storage_root(argv[2]);
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) == 0);
    auto *surface = SDL_CreateRGBSurfaceWithFormat(0, 640, 480, 32, SDL_PIXELFORMAT_RGBA32);
    auto *renderer = SDL_CreateSoftwareRenderer(surface);
    assert(renderer);
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
        SM5XX *running_cpu = app.cpu();
        assert(!app.handle_input({CTL_B, CTL_B}));
        assert(app.cpu() == running_cpu && !app.in_menu());
        assert(!app.handle_input({CTL_L1 | CTL_B, CTL_B}));
        assert(app.in_menu() && app.playing());
        assert(!app.handle_input({CTL_DPAD_RIGHT, CTL_DPAD_RIGHT}));
        assert(app.selected_index() == 0 && !app.playing());
        assert(!app.handle_input({CTL_B, CTL_B}));
        assert(app.in_menu());
        assert(!app.handle_input({CTL_A, CTL_A}));
        assert(!app.in_menu() && app.audio_ready());
        app.return_to_menu();
        assert(!app.handle_input({CTL_X, CTL_X}));
        assert(!app.in_menu() && app.cpu() && app.audio_ready());
        assert(app.handle_input({CTL_START | CTL_SELECT, 0}));
        app.return_to_menu();
        app.select(0);
        app.start(true);
        app.tick(0);
    }
    check_panels(renderer, surface);
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
    std::puts("PASS: pack, PNG, menu, CPU, inputs, SDL audio, render, save/load, switch/resume, controller polling, SDL lifecycle, dual displays/panels/fallback");
}
