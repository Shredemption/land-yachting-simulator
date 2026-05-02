#pragma once

#include <glm/glm.hpp>
#include <map>
#include <string>
#include <vector>
#include <cstdint>

#include "render/render_defs.h"

class FontAtlas
{
public:
    struct TextQuad
    {
        glm::vec2 position;
        glm::vec2 size;
        glm::vec2 uv0;
        glm::vec2 uv1;
    };

    FontAtlas();
    ~FontAtlas();

    bool initialize(const std::string &fontPath, float size);
    void cleanup();
    bool isInitialized() const noexcept;

    const Character *getCharacter(char c) const noexcept;
    float calculateTextWidth(const std::string &text, float scale) const;
    float lineHeight() const noexcept;

    const std::vector<uint8_t> &atlasPixels() const noexcept;
    int atlasWidth() const noexcept;
    int atlasHeight() const noexcept;

private:
    std::map<char, Character> characters;
    std::vector<uint8_t> pixels;
    int width = 0;
    int height = 0;
    int lineHeightPixels = 0;
    int referenceLineHeightPixels = 0;
    float size = 0.0f;
    bool initialized = false;
};
