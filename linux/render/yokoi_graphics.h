#pragma once

#include <SDL.h>
#include <memory>
#include <string>

namespace yokoi
{
    using Texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;

    Texture load_texture(SDL_Renderer *renderer, const std::string &path, int tint = -1);
    void copy_texture(SDL_Renderer *renderer, SDL_Texture *texture,
                      SDL_Rect source, const SDL_FRect &destination);
    int text_width(const std::string &value);
    void draw_text(SDL_Renderer *renderer, float x, float y,
                   const std::string &value, float scale = 2);
}
