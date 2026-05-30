#pragma once

#include <string>
#include <vector>

#include "render/i_renderer.hpp"
#include "render/font_atlas.hpp"

enum class EngineState;

class BaseRenderer : public IRenderer
{
public:
    BaseRenderer();
    ~BaseRenderer() override;

protected:
    bool ensureFontAtlas();
    const FontAtlas &getFontAtlas() const;
    float calculateTextWidth(const std::string &text, float scale) const;

    struct TextQuad
    {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec2 uv0;
        glm::vec2 uv1;
    };

    std::vector<TextQuad> buildTextQuads(const std::string &text, float x, float y, float scale, TextAlign textAlign) const;

    void buildMenu(EngineState state);

private:
    FontAtlas fontAtlas;
    std::string fontPath = "resources/fonts/MusticaPro-SemiBold.otf";
    float textTextureSize = 128.0f;
};
