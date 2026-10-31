#pragma once

#include <SDL.h>
#include <memory>
#include <map>
#include <string>

namespace yokoi
{
    using Texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;

    Texture load_texture(SDL_Renderer *renderer, const std::string &path, int tint = -1);
    void copy_texture(SDL_Renderer *renderer, SDL_Texture *texture,
                      SDL_Rect source, const SDL_FRect &destination);
    float text_width(const std::string &value);
    const char *font_license();

    class Font
    {
    public:
        explicit Font(SDL_Renderer *renderer) : renderer_(renderer) {}
        void draw(float x, float y, const std::string &value, float scale = 2, uint32_t color = 0xffffff);

    private:
        struct Glyph
        {
            Texture texture{nullptr, SDL_DestroyTexture};
            int x = 0, y = 0, width = 0, height = 0;
        };
        Glyph &glyph(int codepoint);
        SDL_Renderer *renderer_;
        std::map<int, Glyph> glyphs_;
    };
}
