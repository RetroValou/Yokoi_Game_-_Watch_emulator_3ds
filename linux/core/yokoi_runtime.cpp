#include "core/yokoi_runtime.h"

#include <SDL.h>
#include <memory>
#include <stdexcept>
#include <string>
#include "core/yokoi_app.h"
#include "core/yokoi_sdl.h"
#include "input/yokoi_controller.h"
#include "render/yokoi_render.h"
#include "std/gw_pack.h"
#include "std/platform_paths.h"

namespace yokoi
{
    void run_application(const char *pack_path, const char *storage_root)
    {
        std::string error;
        set_storage_root(storage_root);
        if (!gw_pack::load(pack_path, &error))
            throw std::runtime_error(error);
        if (!gw_pack::game_count() || gw_pack::game_count() > 255)
            throw std::runtime_error("Pack must contain 1..255 games");
        require_sdl(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) == 0, "SDL init");
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        {
            // Destroy controllers, sessions and textures before the renderer/window and SDL.
            using Window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
            using Renderer = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;
            Window window(SDL_CreateWindow("Yokoi", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                           logical_width, logical_height, SDL_WINDOW_FULLSCREEN_DESKTOP),
                          SDL_DestroyWindow);
            require_sdl(window != nullptr, "Window");
            Renderer renderer(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED),
                              SDL_DestroyRenderer);
            if (!renderer)
                renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_SOFTWARE));
            require_sdl(renderer != nullptr, "Renderer");
            SDL_RenderSetLogicalSize(renderer.get(), logical_width, logical_height);
            App app(renderer.get());
            app.select(0);
            app.open_audio();
            Controller controller;
            controller.open();
            bool quit = false;
            auto deadline = SDL_GetPerformanceCounter();
            const auto frequency = SDL_GetPerformanceFrequency();
            while (!quit)
            {
                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    if (event.type == SDL_QUIT)
                        quit = true;
                    controller.handle_event(event);
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        if (app.in_menu())
                            quit = true;
                        else
                            app.return_to_menu();
                    }
                }
                const auto state = controller.poll();
                if (app.handle_input(state))
                    quit = true;
                app.tick(state.held);
                app.draw();
                // Keep the original 60 Hz pacing; discard lateness rather than catching up.
                deadline += frequency / 60;
                const auto now = SDL_GetPerformanceCounter();
                if (deadline > now)
                    SDL_Delay(Uint32((deadline - now) * 1000 / frequency));
                else
                    deadline = now;
            }
            app.save();
        }
        SDL_Quit();
        gw_pack::unload();
    }
}
