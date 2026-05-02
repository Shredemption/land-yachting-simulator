#include "pch.h"
#include "render/base_renderer.hpp"
#include "render/font_atlas.hpp"

BaseRenderer::BaseRenderer() = default;
BaseRenderer::~BaseRenderer() = default;

bool BaseRenderer::ensureFontAtlas()
{
    if (fontAtlas.isInitialized())
        return true;

    return fontAtlas.initialize(fontPath, textTextureSize);
}

const FontAtlas &BaseRenderer::getFontAtlas() const
{
    return fontAtlas;
}

float BaseRenderer::calculateTextWidth(const std::string &text, float scale) const
{
    return fontAtlas.calculateTextWidth(text, scale);
}

std::vector<BaseRenderer::TextQuad> BaseRenderer::buildTextQuads(const std::string &text, float x, float y, float scale, TextAlign textAlign) const
{
    std::vector<TextQuad> quads;
    if (!fontAtlas.isInitialized())
        return quads;

    std::vector<std::string> lines;
    std::string currentLine;
    for (char c : text)
    {
        if (c == '\n')
        {
            lines.push_back(currentLine);
            currentLine.clear();
            continue;
        }
        currentLine.push_back(c);
    }
    lines.push_back(currentLine);

    float lineSpacing = fontAtlas.lineHeight() * scale * 1.5f;
    float cursorY = y;

    for (const std::string &line : lines)
    {
        float lineWidth = fontAtlas.calculateTextWidth(line, scale);
        float cursorX = x;

        if (textAlign != TextAlign::Left)
        {
            if (textAlign == TextAlign::Center)
                cursorX -= lineWidth * 0.5f;
            else
                cursorX -= lineWidth;
        }

        for (char c : line)
        {
            const Character *ch = fontAtlas.getCharacter(c);
            if (!ch)
                continue;

            float xpos = cursorX + ch->Bearing.x * scale;
            float ypos = cursorY + (fontAtlas.lineHeight() - ch->Bearing.y) * scale;
            float w = ch->Size.x * scale;
            float h = ch->Size.y * scale;

            TextQuad quad;
            quad.position = glm::vec2(xpos, ypos);
            quad.size = glm::vec2(w, h);
            quad.uv0 = glm::vec2(ch->TexCoords.x, ch->TexCoords.y);
            quad.uv1 = glm::vec2(ch->TexCoords.x + ch->TexCoords.z, ch->TexCoords.y + ch->TexCoords.w);
            quads.push_back(quad);

            cursorX += (ch->Advance >> 6) * scale;
        }

        cursorY += lineSpacing;
    }

    return quads;
}
