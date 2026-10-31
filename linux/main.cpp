// Linux/PortMaster entry point; frontend behavior lives in core/, input/ and render/.
#include <SDL.h>
#include <cstdio>
#include <exception>
#include <string>
#include "core/yokoi_runtime.h"

int main(int argc, char **argv)
{
    if (argc > 1 && std::string(argv[1]) == "--help")
    {
        std::puts("Yokoi Linux: yokoi [Android ROM pack.ykp] [storage directory]\n"
                  "Default pack: ./yokoi_pack_rgds.ykp\n"
                  "Keyboard: arrows, Z=A X=B A=Y S=X, Enter=Start, Tab=Select, Q=L, Escape=menu\n"
                  "Menu: arrows choose; Z resume; S new game; Start+Select exits.");
        return 0;
    }
    try
    {
        yokoi::run_application(argc > 1 ? argv[1] : "yokoi_pack_rgds.ykp",
                               argc > 2 ? argv[2] : ".");
        return 0;
    }
    catch (const std::exception &error)
    {
        std::fprintf(stderr, "Yokoi: %s\n", error.what());
        if (SDL_WasInit(SDL_INIT_VIDEO))
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Yokoi", error.what(), nullptr);
        SDL_Quit();
        return 1;
    }
}
