// Linux/PortMaster entry point; frontend behavior lives in core/, input/ and render/.
#include <SDL.h>
#include <cstdio>
#include <exception>
#include <string>
#include "core/yokoi_runtime.h"
#include "render/yokoi_graphics.h"

int main(int argc, char **argv)
{
    if (argc > 1 && std::string(argv[1]) == "--font-license")
    {
        std::puts(yokoi::font_license());
        return 0;
    }
    if (argc > 1 && std::string(argv[1]) == "--help")
    {
        std::puts("Yokoi Linux: yokoi [Android ROM pack.ykp] [storage directory]\n"
                  "Default pack: ./yokoi_pack_rgds.ykp\n"
                  "Keyboard: arrows, Z=A X=B A=Y S=X, Enter=Start, Tab=Select, Q=L, R/F1/Escape=settings\n"
                  "Menu: left/right game, up/down manufacturer; Z opens fresh/save choice; X goes back.\n"
                  "Settings: up/down row, left/right change, Z activate; Start+Select saves and exits.\n"
                  "Selection settings also offer Exit App.\n"
                  "Font: embedded Audiowide (SIL OFL 1.1); --font-license prints its licence.");
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
