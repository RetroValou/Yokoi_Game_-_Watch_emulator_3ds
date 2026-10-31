#include "render/yokoi_graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "third_party/stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb_truetype.h"
#include <climits>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include "core/yokoi_sdl.h"
#include "std/gw_pack.h"

extern "C" const unsigned char yokoi_font_data[];
extern "C" const char yokoi_font_license[];

namespace yokoi
{
    namespace
    {
        constexpr float glyph_height = 64;
        constexpr float text_height = 12;

        const stbtt_fontinfo &font_info()
        {
            static const auto info = [] {
                stbtt_fontinfo result{};
                if (!stbtt_InitFont(&result, yokoi_font_data, 0))
                    throw std::runtime_error("Invalid embedded Audiowide font");
                return result;
            }();
            return info;
        }

        std::vector<int> codepoints(const std::string &value)
        {
            std::vector<int> result;
            for (size_t offset = 0; offset < value.size();)
            {
                uint32_t codepoint = static_cast<unsigned char>(value[offset++]);
                if (!codepoint)
                    break;
                size_t remaining = 0;
                uint32_t minimum = 0;
                if (codepoint >= 0x80)
                {
                    if ((codepoint & 0xe0) == 0xc0)
                    {
                        remaining = 1;
                        minimum = 0x80;
                        codepoint &= 0x1f;
                    }
                    else if ((codepoint & 0xf0) == 0xe0)
                    {
                        remaining = 2;
                        minimum = 0x800;
                        codepoint &= 0x0f;
                    }
                    else if ((codepoint & 0xf8) == 0xf0)
                    {
                        remaining = 3;
                        minimum = 0x10000;
                        codepoint &= 0x07;
                    }
                    else
                        throw std::runtime_error("Invalid UTF-8 menu text");
                }
                if (remaining > value.size() - offset)
                    throw std::runtime_error("Truncated UTF-8 menu text");
                for (size_t i = 0; i < remaining; ++i)
                {
                    const auto byte = static_cast<unsigned char>(value[offset++]);
                    if ((byte & 0xc0) != 0x80)
                        throw std::runtime_error("Invalid UTF-8 continuation in menu text");
                    codepoint = (codepoint << 6) | (byte & 0x3f);
                }
                if (codepoint < minimum || codepoint > 0x10ffff ||
                    (codepoint >= 0xd800 && codepoint <= 0xdfff))
                    throw std::runtime_error("Invalid Unicode codepoint in menu text");
                result.push_back(int(codepoint));
            }
            return result;
        }
    }

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

    float text_width(const std::string &value)
    {
        const auto &info = font_info();
        const float factor = stbtt_ScaleForPixelHeight(&info, text_height);
        float width = 0, widest = 0;
        int previous = 0;
        for (int codepoint : codepoints(value))
        {
            if (codepoint == '\n')
            {
                widest = std::max(widest, width);
                width = 0;
                previous = 0;
                continue;
            }
            int advance;
            stbtt_GetCodepointHMetrics(&info, codepoint, &advance, nullptr);
            width += (advance + stbtt_GetCodepointKernAdvance(&info, previous, codepoint)) * factor;
            previous = codepoint;
        }
        return std::max(widest, width);
    }

    const char *font_license() { return yokoi_font_license; }

    Font::Glyph &Font::glyph(int codepoint)
    {
        auto [entry, inserted] = glyphs_.try_emplace(codepoint);
        auto &result = entry->second;
        if (!inserted)
            return result;
        const auto &info = font_info();
        if (!stbtt_FindGlyphIndex(&info, codepoint))
            std::fprintf(stderr, "Audiowide has no glyph for U+%04X; displaying its missing-glyph symbol.\n",
                         unsigned(codepoint));
        const float factor = stbtt_ScaleForPixelHeight(&info, glyph_height);
        std::unique_ptr<unsigned char, decltype(&std::free)> bitmap(
            stbtt_GetCodepointBitmap(&info, factor, factor, codepoint,
                &result.width, &result.height, &result.x, &result.y), std::free);
        if (result.width > 0 && result.height > 0)
        {
            if (!bitmap)
                throw std::runtime_error("Cannot rasterize Audiowide glyph");
            std::vector<unsigned char> pixels(size_t(result.width) * result.height * 4, 255);
            for (size_t i = 0; i < size_t(result.width) * result.height; ++i)
                pixels[i * 4 + 3] = bitmap.get()[i];
            SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(), result.width,
                result.height, 32, result.width * 4, SDL_PIXELFORMAT_RGBA32);
            require_sdl(surface != nullptr, "Create font surface");
            result.texture.reset(SDL_CreateTextureFromSurface(renderer_, surface));
            SDL_FreeSurface(surface);
            require_sdl(result.texture != nullptr, "Create font texture");
            require_sdl(SDL_SetTextureBlendMode(result.texture.get(), SDL_BLENDMODE_BLEND) == 0,
                        "Font blending");
        }
        return result;
    }

    void Font::draw(float x, float y, const std::string &value, float scale, uint32_t color)
    {
        const auto &info = font_info();
        const float factor = stbtt_ScaleForPixelHeight(&info, text_height) * scale;
        const float bitmap_scale = text_height * scale / glyph_height;
        int ascent;
        stbtt_GetFontVMetrics(&info, &ascent, nullptr, nullptr);
        const float start = x;
        y += ascent * factor;
        int previous = 0;
        for (int codepoint : codepoints(value))
        {
            if (codepoint == '\n')
            {
                x = start;
                y += text_height * scale;
                previous = 0;
                continue;
            }
            x += stbtt_GetCodepointKernAdvance(&info, previous, codepoint) * factor;
            auto &image = glyph(codepoint);
            if (image.texture)
            {
                SDL_FRect destination{x + image.x * bitmap_scale, y + image.y * bitmap_scale,
                                      image.width * bitmap_scale, image.height * bitmap_scale};
                require_sdl(SDL_SetTextureColorMod(image.texture.get(), (color >> 16) & 255,
                            (color >> 8) & 255, color & 255) == 0, "Font color");
                require_sdl(SDL_RenderCopyF(renderer_, image.texture.get(), nullptr, &destination) == 0,
                            "Draw font glyph");
            }
            int advance;
            stbtt_GetCodepointHMetrics(&info, codepoint, &advance, nullptr);
            x += advance * factor;
            previous = codepoint;
        }
    }
}
