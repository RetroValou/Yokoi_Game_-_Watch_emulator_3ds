#include "render/yokoi_graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "third_party/stb_image.h"
#include "third_party/stb_easy_font.h"
#include <climits>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include "core/yokoi_sdl.h"
#include "std/gw_pack.h"

namespace yokoi
{
    Texture load_texture(SDL_Renderer *renderer, const std::string &path, int tint)
    {
        const uint8_t *bytes = nullptr;
        size_t size = 0;
        // ROM metadata uses gfx paths; the packed PNG directory is keyed by basename.
        if (!gw_pack::get_file_bytes(std::filesystem::path(path).filename().string(), bytes, size) ||
            size > INT_MAX)
            throw std::runtime_error("Missing PNG in Android pack: " + path);
        int width, height, channels;
        stbi_uc *pixels = stbi_load_from_memory(bytes, static_cast<int>(size),
                                               &width, &height, &channels, 4);
        if (!pixels)
            throw std::runtime_error("Cannot decode PNG: " + path);
        if (tint >= 0)
        {
            for (size_t i = 0; i < size_t(width) * height; ++i)
            {
                pixels[i * 4] = (tint >> 16) & 255;
                pixels[i * 4 + 1] = (tint >> 8) & 255;
                pixels[i * 4 + 2] = tint & 255;
            }
        }
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
            pixels, width, height, 32, width * 4, SDL_PIXELFORMAT_RGBA32);
        SDL_Texture *result = surface ? SDL_CreateTextureFromSurface(renderer, surface) : nullptr;
        SDL_FreeSurface(surface);
        stbi_image_free(pixels);
        require_sdl(result != nullptr, "Create texture");
        SDL_SetTextureBlendMode(result, SDL_BLENDMODE_BLEND);
        return Texture(result, SDL_DestroyTexture);
    }

    void copy_texture(SDL_Renderer *renderer, SDL_Texture *texture,
                      SDL_Rect source, const SDL_FRect &destination)
    {
        // Android packs store bottom-origin atlas rectangles and mirrored artwork.
        int height = 0;
        SDL_QueryTexture(texture, nullptr, nullptr, nullptr, &height);
        source.y = height - source.y - source.h;
        SDL_RenderCopyExF(renderer, texture, &source, &destination, 0, nullptr, SDL_FLIP_HORIZONTAL);
    }

    int text_width(const std::string &value)
    {
        return stb_easy_font_width(const_cast<char *>(value.c_str()));
    }

    void draw_text(SDL_Renderer *renderer, float x, float y, const std::string &value, float scale)
    {
        std::vector<unsigned char> buffer(value.size() * 300 + 1024);
        const int quads = stb_easy_font_print(0, 0, const_cast<char *>(value.c_str()),
                                              nullptr, buffer.data(), int(buffer.size()));
        struct Vertex
        {
            float x, y, z;
            unsigned char color[4];
        };
        const auto *vertices = reinterpret_cast<const Vertex *>(buffer.data());
        SDL_SetRenderDrawColor(renderer, 235, 237, 240, 255);
        for (int i = 0; i < quads; ++i)
        {
            SDL_FRect rect{x + vertices[i * 4].x * scale, y + vertices[i * 4].y * scale,
                           (vertices[i * 4 + 2].x - vertices[i * 4].x) * scale,
                           (vertices[i * 4 + 2].y - vertices[i * 4].y) * scale};
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
}
