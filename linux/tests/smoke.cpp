#include <SDL.h>
#include <cassert>
#include <cstdio>
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
    std::puts("PASS: pack, PNG, menu, CPU, inputs, SDL audio, render, save/load, switch/resume, controller polling, SDL lifecycle");
}
