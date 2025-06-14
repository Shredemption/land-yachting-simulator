#include "GLGPUDriver.h"
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "render/render.hpp"
#include "shader/shader_util.hpp"
#include "shader/shaderID.h"

using namespace ultralight;

glm::mat4 ToGLM(const ultralight::Matrix4x4 &mat)
{
    glm::mat4 out;
    std::memcpy(glm::value_ptr(out), mat.data, sizeof(float) * 16);
    return out;
}

glm::vec4 ToGLM(const ultralight::vec4 &vec)
{
    glm::vec4 out;
    out.x = vec.x;
    out.y = vec.y;
    out.z = vec.z;
    out.w = vec.w;
    return out;
}

GLGPUDriver::GLGPUDriver() {}
GLGPUDriver::~GLGPUDriver()
{
    for (auto &[id, tex] : textures_)
        glDeleteTextures(1, &tex);
    for (auto &[id, vao] : geometries_)
        glDeleteVertexArrays(1, &vao);
}

uint32_t GLGPUDriver::NextTextureId()
{
    return next_texture_id_++;
}

void GLGPUDriver::CreateTexture(uint32_t id, RefPtr<Bitmap> bitmap)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    auto format = bitmap->format();
    GLenum internal_format = GL_RGBA8;
    GLenum data_format = GL_BGRA;

    if (format == BitmapFormat::A8_UNORM)
    {
        internal_format = GL_R8;
        data_format = GL_RED;
    }

    void *pixels = bitmap->LockPixels();
    if (!pixels)
    {
        return;
    }

    int bytes_per_pixel = (format == BitmapFormat::A8_UNORM) ? 1 : 4;
    int stride = bitmap->row_bytes(); // or similar, check your Bitmap API
    if (stride != bitmap->width() * bytes_per_pixel)
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, stride / bytes_per_pixel);
    }
    else
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format,
                 bitmap->width(), bitmap->height(), 0,
                 data_format, GL_UNSIGNED_BYTE, pixels);
    bitmap->UnlockPixels();

    textures_[id] = tex;
    bitmap_textures_[bitmap.get()] = tex;
}

void GLGPUDriver::UpdateTexture(uint32_t id, RefPtr<Bitmap> bitmap)
{
    auto it = textures_.find(id);
    if (it == textures_.end())
        return;

    GLuint tex = it->second;
    glBindTexture(GL_TEXTURE_2D, tex);

    auto format = bitmap->format();
    GLenum data_format = (format == BitmapFormat::A8_UNORM) ? GL_RED : GL_BGRA;

    void *pixels = bitmap->LockPixels();
    if (!pixels)
    {
        return;
    }

    int bytes_per_pixel = (format == BitmapFormat::A8_UNORM) ? 1 : 4;
    int stride = bitmap->row_bytes(); // or similar, check your Bitmap API
    if (stride != bitmap->width() * bytes_per_pixel)
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, stride / bytes_per_pixel);
    }
    else
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bitmap->width(), bitmap->height(),
                    data_format, GL_UNSIGNED_BYTE, pixels);
    bitmap->UnlockPixels();

    bitmap_textures_[bitmap.get()] = tex;
}

void GLGPUDriver::DestroyTexture(uint32_t id)
{
    auto it = textures_.find(id);
    if (it != textures_.end())
    {
        glDeleteTextures(1, &it->second);
        textures_.erase(it);
    }
}

uint32_t GLGPUDriver::NextRenderBufferId()
{
    return next_renderbuffer_id_++;
}

void GLGPUDriver::CreateRenderBuffer(uint32_t id, const ultralight::RenderBuffer &buf)
{
    // Stub: Ultralight handles its own internal framebuffer; no-op unless integrating custom FBOs
}

void GLGPUDriver::DestroyRenderBuffer(uint32_t id)
{
    // Stub: same as above
}

uint32_t GLGPUDriver::NextGeometryId()
{
    return next_geometry_id_++;
}

void GLGPUDriver::CreateGeometry(uint32_t id, const VertexBuffer &vb, const IndexBuffer &ib)
{
    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    if (vb.format == VertexBufferFormat::_2f_4ub_2f_2f_28f)
    {
        glBufferData(GL_ARRAY_BUFFER, vb.size, vb.data, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // pos
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, pos));
        glEnableVertexAttribArray(1); // color
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, color));
        glEnableVertexAttribArray(2); // tex
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, tex));
        glEnableVertexAttribArray(3); // obj
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, obj));
        glEnableVertexAttribArray(4); // obj
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, data0));
        glEnableVertexAttribArray(5); // obj
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, data1));
        glEnableVertexAttribArray(6); // obj
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, data2));
        glEnableVertexAttribArray(7); // obj
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, data3));
        glEnableVertexAttribArray(8); // obj
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, data4));
        glEnableVertexAttribArray(9); // obj
        glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, data5));
        glEnableVertexAttribArray(10); // obj
        glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f_2f_28f), (void *)offsetof(Vertex_2f_4ub_2f_2f_28f, data6));
    }
    else if (vb.format == VertexBufferFormat::_2f_4ub_2f)
    {
        glBufferData(GL_ARRAY_BUFFER, vb.size * sizeof(Vertex_2f_4ub_2f), vb.data, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // pos
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_2f_4ub_2f), (void *)offsetof(Vertex_2f_4ub_2f, pos));
        glEnableVertexAttribArray(1); // color
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_2f_4ub_2f), (void *)offsetof(Vertex_2f_4ub_2f, color));
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ib.size, ib.data, GL_STATIC_DRAW);

    geometries_[id] = vao;
    glBindVertexArray(0);
}

void GLGPUDriver::UpdateGeometry(uint32_t id, const VertexBuffer &vb, const IndexBuffer &ib)
{
    DestroyGeometry(id);
    CreateGeometry(id, vb, ib);
}

void GLGPUDriver::DestroyGeometry(uint32_t id)
{
    auto it = geometries_.find(id);
    if (it != geometries_.end())
    {
        glDeleteVertexArrays(1, &it->second);
        geometries_.erase(it);
    }
}

void GLGPUDriver::UpdateCommandList(const ultralight::CommandList &command_list)
{
    bool fboCleared = false;

    glBindFramebuffer(GL_FRAMEBUFFER, Render::htmlFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    auto &commands = command_list.commands;
    uint32_t num_commands = command_list.size;

    for (size_t i = 0; i < num_commands; ++i)
    {
        const auto &cmd = commands[i];
        switch (cmd.command_type)
        {
        case ultralight::CommandType::DrawGeometry:
        {
            glm::mat4 ortho = glm::ortho(
                0.0f,
                static_cast<float>(cmd.gpu_state.viewport_width),
                static_cast<float>(cmd.gpu_state.viewport_height),
                0.0f, // Y-down coordinate system!
                -1.0f,
                1.0f);

            Shader *shader;
            switch (cmd.gpu_state.shader_type)
            {
            case ShaderType::Fill:
                shader = ShaderUtil::load(shaderID::Fill);
                break;

            case ShaderType::FillPath:
                shader = ShaderUtil::load(shaderID::FillPath);
                break;
            }

            GLint loc_State = glGetUniformLocation(shader->m_id, "State");
            GLint loc_Transform = glGetUniformLocation(shader->m_id, "Transform");
            GLint loc_Scalar4 = glGetUniformLocation(shader->m_id, "Scalar4");
            GLint loc_Vector = glGetUniformLocation(shader->m_id, "Vector");
            GLint loc_ClipSize = glGetUniformLocation(shader->m_id, "ClipSize");
            GLint loc_Clip = glGetUniformLocation(shader->m_id, "Clip");

            glm::vec4 state = {
                cmd.gpu_state.enable_texturing ? 1.0f : 0.0f,
                cmd.gpu_state.enable_blend ? 1.0f : 0.0f,
                0.0f, 0.0f};

            glm::mat4 finalTransform = ortho * ToGLM(cmd.gpu_state.transform);

            loc_State = glGetUniformLocation(shader->m_id, "State");
            loc_Transform = glGetUniformLocation(shader->m_id, "Transform");
            loc_Scalar4 = glGetUniformLocation(shader->m_id, "Scalar4");
            loc_Vector = glGetUniformLocation(shader->m_id, "Vector");
            loc_ClipSize = glGetUniformLocation(shader->m_id, "ClipSize");
            loc_Clip = glGetUniformLocation(shader->m_id, "Clip");

            glUniform4fv(loc_State, 1, glm::value_ptr(state));
            glUniformMatrix4fv(loc_Transform, 1, GL_FALSE, glm::value_ptr(finalTransform));
            glUniform4fv(loc_Scalar4, 1, &cmd.gpu_state.uniform_scalar[0]);
            glUniform4fv(loc_Scalar4 + 1, 1, &cmd.gpu_state.uniform_scalar[4]);
            glUniform4fv(loc_Vector, 8, (const float *)&cmd.gpu_state.uniform_vector[0]);
            glUniform1ui(loc_ClipSize, cmd.gpu_state.clip_size);
            glUniformMatrix4fv(loc_Clip, 8, GL_FALSE, (const float *)&cmd.gpu_state.clip[0]);

            shader->setInt("Texture1", 1);
            shader->setInt("Texture2", 2);
            shader->setInt("Texture3", 3);

            // Get VAO from geometry ID
            auto it_vao = geometries_.find(cmd.geometry_id);
            if (it_vao == geometries_.end())
                break; // invalid geometry id

            GLuint vao = it_vao->second;

            // Bind VAO
            glBindVertexArray(vao);

            // Bind texture
            if (cmd.gpu_state.enable_texturing)
            {
                if (cmd.gpu_state.texture_1_id != 0)
                {
                    auto it_tex = textures_.find(cmd.gpu_state.texture_1_id);
                    if (it_tex != textures_.end())
                    {
                        glActiveTexture(GL_TEXTURE0 + 1);
                        glBindTexture(GL_TEXTURE_2D, it_tex->second);
                    }
                }

                if (cmd.gpu_state.texture_2_id != 0)
                {
                    auto it_tex = textures_.find(cmd.gpu_state.texture_2_id);
                    if (it_tex != textures_.end())
                    {
                        glActiveTexture(GL_TEXTURE0 + 2);
                        glBindTexture(GL_TEXTURE_2D, it_tex->second);
                    }
                }

                if (cmd.gpu_state.texture_3_id != 0)
                {
                    auto it_tex = textures_.find(cmd.gpu_state.texture_3_id);
                    if (it_tex != textures_.end())
                    {
                        glActiveTexture(GL_TEXTURE0 + 3);
                        glBindTexture(GL_TEXTURE_2D, it_tex->second);
                    }
                }
            }

            // Set scissor rect
            if (cmd.gpu_state.scissor_rect.width() > 0 && cmd.gpu_state.scissor_rect.height() > 0)
            {
                glEnable(GL_SCISSOR_TEST);
                glScissor(cmd.gpu_state.scissor_rect.x(), cmd.gpu_state.scissor_rect.y(), cmd.gpu_state.scissor_rect.width(), cmd.gpu_state.scissor_rect.height());
            }
            else
            {
                glDisable(GL_SCISSOR_TEST);
            }

            if (cmd.gpu_state.enable_scissor)
            {
                glEnable(GL_SCISSOR_TEST);
                const auto &r = cmd.gpu_state.scissor_rect;
                glScissor(r.x(), r.y(), r.width(), r.height());
            }
            else
            {
                glDisable(GL_SCISSOR_TEST);
            }

            if (cmd.gpu_state.enable_blend)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // assuming premultiplied alpha
            }
            else
            {
                glDisable(GL_BLEND);
            }

            glDisable(GL_DEPTH_TEST);
            // Draw elements
            glDrawElements(GL_TRIANGLES, cmd.indices_count, GL_UNSIGNED_INT, (void *)(cmd.indices_offset * sizeof(uint32_t)));

            // Cleanup
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_SCISSOR_TEST);
            glEnable(GL_DEPTH_TEST);

            break;
        }

        case ultralight::CommandType::ClearRenderBuffer:
        {
            if (!fboCleared)
            {
                glClearColor(0, 0, 0, 1);
                glClear(GL_COLOR_BUFFER_BIT);

                fboCleared = true;
            }
            break;
        }

        default:
            break;
        }
    }
}

GLuint GLGPUDriver::GetTextureForBitmap(Bitmap *bitmap)
{
    auto it = bitmap_textures_.find(bitmap);
    if (it != bitmap_textures_.end())
        return it->second;
    return 0; // Not found
}
