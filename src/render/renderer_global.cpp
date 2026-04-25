#include "renderer_global.hpp"

std::unique_ptr<IRenderer> g_renderer = nullptr;

IRenderer &Renderer()
{
    return *g_renderer;
}

IRenderer *RendererPtr()
{
    return g_renderer.get();
}