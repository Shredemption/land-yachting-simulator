#include "pch.h"
#include "render/font_atlas.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

FontAtlas::FontAtlas() = default;
FontAtlas::~FontAtlas() { cleanup(); }

bool FontAtlas::initialize(const std::string &fontPath, float size)
{
    cleanup();

    FT_Library ft;
    FT_Face face;

    if (FT_Init_FreeType(&ft))
        return false;

    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        FT_Done_FreeType(ft);
        return false;
    }

    if (FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(size)))
    {
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return false;
    }

    const int atlasSize = 1024;
    const int gapX = 2;
    const int gapY = 2;
    width = atlasSize;
    height = atlasSize;
    pixels.assign(width * height * 4, 0);

    int xPos = 0;
    int yPos = 0;
    int rowHeight = 0;

    for (unsigned char c = 0; c < 128; ++c)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        FT_GlyphSlot glyph = face->glyph;
        int glyphWidth = glyph->bitmap.width;
        int glyphHeight = glyph->bitmap.rows;

        if (xPos + glyphWidth + gapX >= width)
        {
            xPos = 0;
            yPos += rowHeight + gapY;
            rowHeight = 0;
        }

        if (yPos + glyphHeight + gapY >= height)
            break;

        for (int row = 0; row < glyphHeight; ++row)
        {
            for (int col = 0; col < glyphWidth; ++col)
            {
                int atlasIndex = ((yPos + row) * width + (xPos + col)) * 4;
                uint8_t value = glyph->bitmap.buffer[row * glyphWidth + col];
                pixels[atlasIndex + 0] = 255;
                pixels[atlasIndex + 1] = 255;
                pixels[atlasIndex + 2] = 255;
                pixels[atlasIndex + 3] = value;
            }
        }

        Character ch;
        ch.Size = glm::ivec2(glyphWidth, glyphHeight);
        ch.Bearing = glm::ivec2(glyph->bitmap_left, glyph->bitmap_top);
        ch.Advance = static_cast<unsigned int>(glyph->advance.x);
        ch.TexCoords = glm::vec4(
            static_cast<float>(xPos) / static_cast<float>(width),
            static_cast<float>(yPos) / static_cast<float>(height),
            static_cast<float>(glyphWidth) / static_cast<float>(width),
            static_cast<float>(glyphHeight) / static_cast<float>(height));

        characters[c] = ch;
        lineHeightPixels = std::max(lineHeightPixels, glyphHeight);
        if (c == 'H')
            referenceLineHeightPixels = glyphHeight;

        xPos += glyphWidth + gapX;
        rowHeight = std::max(rowHeight, glyphHeight);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    this->size = size;
    if (lineHeightPixels == 0)
        lineHeightPixels = static_cast<int>(size);
    if (referenceLineHeightPixels == 0)
        referenceLineHeightPixels = lineHeightPixels;
    initialized = true;
    return true;
}

void FontAtlas::cleanup()
{
    characters.clear();
    pixels.clear();
    width = 0;
    height = 0;
    lineHeightPixels = 0;
    referenceLineHeightPixels = 0;
    size = 0.0f;
    initialized = false;
}

bool FontAtlas::isInitialized() const noexcept
{
    return initialized;
}

const Character *FontAtlas::getCharacter(char c) const noexcept
{
    auto it = characters.find(c);
    return it == characters.end() ? nullptr : &it->second;
}

float FontAtlas::calculateTextWidth(const std::string &text, float scale) const
{
    float width = 0.0f;
    float lineWidth = 0.0f;

    for (char c : text)
    {
        if (c == '\n')
        {
            width = std::max(width, lineWidth);
            lineWidth = 0.0f;
            continue;
        }

        const Character *ch = getCharacter(c);
        if (!ch)
            continue;

        lineWidth += (ch->Advance >> 6) * scale;
    }

    return std::max(width, lineWidth);
}

float FontAtlas::lineHeight() const noexcept
{
    return static_cast<float>(referenceLineHeightPixels);
}

const std::vector<uint8_t> &FontAtlas::atlasPixels() const noexcept
{
    return pixels;
}

int FontAtlas::atlasWidth() const noexcept
{
    return width;
}

int FontAtlas::atlasHeight() const noexcept
{
    return height;
}
