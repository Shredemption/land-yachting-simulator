#pragma once

#include <memory>
#include "i_renderer.hpp"

extern std::unique_ptr<IRenderer> g_renderer;

IRenderer &Renderer();
IRenderer *RendererPtr();