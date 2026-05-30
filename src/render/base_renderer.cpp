#include "base_renderer.hpp"

#include "pch.h"

#include "render/font_atlas.hpp"
#include "ui_manager/ui_manager_defs.h"

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

void BaseRenderer::buildMenu(EngineState state)
{
    float alpha = std::clamp(UIManager::fade / UIManager::fadeTime, 0.0f, 1.0f);

    switch (state)
    {
    case EngineState::Title:
    case EngineState::TitleSettings:
    case EngineState::TestMenu:
    {
        float color = 0.0f;
        if (UIManager::shouldFadeBackground)
            color = easeInOutQuad(0.0f, 0.5f, alpha);
        else
            color = 0.5f;

        setClearColor(color, 0, 0, 1);

        break;
    }
    case EngineState::Pause:
    case EngineState::Settings:
    {
        float darken = 0.0f;
        float darkenOffset = 0.0f;
        if (UIManager::shouldFadeBackground)
            if (UIManager::fadeToBlack)
            {
                darken = easeInOutQuad(1.0f, 0.5f, alpha);
                darkenOffset = easeInOutCirc(1.0f, 0.0f, alpha);
            }
            else
                darken = easeInOutQuad(0.0f, 0.5f, alpha);
        else
            darken = 0.5f;

        drawPauseBackground(darken, darkenOffset);
    }
    }

    float titleX = 0.02f, titleY = 0.04f;
    float shadowDistance = 0.003f;
    std::string titleText;

    switch (state)
    {
    case EngineState::Title:
        titleText = "Land Yachting Simulator";
        break;
    case EngineState::Pause:
        titleText = "Paused";
        break;
    case EngineState::TestMenu:
        titleText = "Tests";
        break;
    case EngineState::Settings:
    case EngineState::TitleSettings:
        titleText = "Settings";
        break;
    }

    float positionOffset = easeInOutQuad(-0.01f, 0.0f, alpha);

    renderText(titleText, titleX + positionOffset + shadowDistance, titleY + shadowDistance, 1.0f, glm::vec3(0.0f), alpha, TextAlign::Left);
    renderText(titleText, titleX + positionOffset, titleY, 1.0f, glm::vec3(1.0f), alpha, TextAlign::Left);

    if (state == EngineState::Title)
    {
        glm::vec2 pos = {0.7f + positionOffset, 0.5f};

        renderImage("title-figure-black.png", pos + glm::vec2(0.005f, -0.01f), 835, 1024, alpha, glm::vec2(1.0f, 1.0f), true);
        renderImage("title-figure.png", pos, 835, 1024, alpha, glm::vec2(1.0f, 1.0f), true);
    }

    if (UIManager::needsRestart)
    {
        renderText("Will restart to apply changes", 0.98f + shadowDistance, titleY + shadowDistance, 1.0f, glm::vec3(0.0f), alpha, TextAlign::Right);
        renderText("Will restart to apply changes", 0.98f, titleY, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f), alpha, TextAlign::Right);
    }
    else if (UIManager::needsReload)
    {
        renderText("Will reload to apply changes", 0.98f + shadowDistance, titleY + shadowDistance, 1.0f, glm::vec3(0.0f), alpha, TextAlign::Right);
        renderText("Will reload to apply changes", 0.98f, titleY, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f), alpha, TextAlign::Right);
    }
}