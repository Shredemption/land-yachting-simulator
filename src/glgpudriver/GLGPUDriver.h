#pragma once
#include <Ultralight/platform/GPUDriver.h>
#include <Ultralight/platform/Platform.h>
#include <Ultralight/Ultralight.h>
#include <unordered_map>

#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

class GLGPUDriver : public ultralight::GPUDriver
{
public:
    GLGPUDriver();
    ~GLGPUDriver();

    uint32_t NextTextureId() override;
    void CreateTexture(uint32_t id, ultralight::RefPtr<ultralight::Bitmap> bitmap) override;
    void UpdateTexture(uint32_t id, ultralight::RefPtr<ultralight::Bitmap> bitmap) override;
    void DestroyTexture(uint32_t id) override;

    uint32_t NextRenderBufferId() override;
    void CreateRenderBuffer(uint32_t id, const ultralight::RenderBuffer &buf) override;
    void DestroyRenderBuffer(uint32_t id) override;

    uint32_t NextGeometryId() override;
    void CreateGeometry(uint32_t id, const ultralight::VertexBuffer &vb, const ultralight::IndexBuffer &ib) override;
    void UpdateGeometry(uint32_t id, const ultralight::VertexBuffer &vb, const ultralight::IndexBuffer &ib) override;
    void DestroyGeometry(uint32_t id) override;

    void BeginSynchronize() override {}
    void EndSynchronize() override {}
    void UpdateCommandList(const ultralight::CommandList &) override;

    GLuint GLGPUDriver::GetTextureForBitmap(ultralight::Bitmap *bitmap);

private:
    uint32_t next_texture_id_ = 1;
    uint32_t next_renderbuffer_id_ = 1;
    uint32_t next_geometry_id_ = 1;

    std::unordered_map<ultralight::Bitmap *, GLuint> bitmap_textures_;
    std::unordered_map<uint32_t, GLuint> textures_;
    std::unordered_map<uint32_t, GLuint> geometries_;
};
