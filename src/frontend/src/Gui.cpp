/* This file is part of Epoch, Copyright (C) 2023 Andrea Ghidini.
 *
 * Epoch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Epoch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Epoch.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Gui.hpp"

#include <cassert>
#include <stdexcept>

#include <imgui.h>

#include "Shader.hpp"

static const char* IMGUI_VERTEX_SHADER = R"GLSL(
#version 410 core

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec4 Color;
uniform mat4 ProjMtx;
out vec2 Frag_UV;
out vec4 Frag_Color;
void main()
{
    Frag_UV = UV;
    Frag_Color = Color;
    gl_Position = ProjMtx * vec4(Position.xy,0,1);
}
)GLSL";

static const char* IMGUI_FRAGMENT_SHADER = R"GLSL(
#version 410 core

in vec2 Frag_UV;
in vec4 Frag_Color;
uniform sampler2D Texture;
layout (location = 0) out vec4 Out_Color;
void main()
{
    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
}
)GLSL";

namespace epoch::frontend
{
    Gui::Gui()
    {
        m_shader = std::make_unique<Shader>(IMGUI_VERTEX_SHADER, IMGUI_FRAGMENT_SHADER);
        m_shader->bind();
        m_shader->setUniformTexture("Texture", 0);

        glGenVertexArrays(1, &m_vao);
        assert(m_vao);
        glGenBuffers(1, &m_vertexBuffer);
        assert(m_vertexBuffer);
        glGenBuffers(1, &m_indexBuffer);
        assert(m_indexBuffer);
        glGenTextures(1, &m_fontTexture);
        assert(m_fontTexture);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(IM_OFFSETOF(ImDrawVert, pos)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(IM_OFFSETOF(ImDrawVert, uv)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(IM_OFFSETOF(ImDrawVert, col)));
        glEnableVertexAttribArray(2);

        m_context = ImGui::CreateContext();
        ImGui::StyleColorsDark();

        const auto& io = ImGui::GetIO();

        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        glBindTexture(GL_TEXTURE_2D, m_fontTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        io.Fonts->SetTexID(reinterpret_cast<void*>(static_cast<intptr_t>(m_fontTexture)));
    }

    Gui::~Gui()
    {
        if (m_context)
        {
            ImGui::DestroyContext(m_context);
        }
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vertexBuffer) glDeleteBuffers(1, &m_vertexBuffer);
        if (m_indexBuffer) glDeleteBuffers(1, &m_indexBuffer);
        if (m_fontTexture) glDeleteTextures(1, &m_fontTexture);
    }

    void Gui::newFrame(const int width, const int height)
    {
        auto& io = ImGui::GetIO();
        io.DisplaySize = ImVec2{ static_cast<float>(width), static_cast<float>(height) };
        ImGui::NewFrame();
    }

    void Gui::render()
    {
        ImGui::Render();
        auto& io = ImGui::GetIO();
        const auto drawData = ImGui::GetDrawData();

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_SCISSOR_TEST);
        glActiveTexture(GL_TEXTURE0);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        m_shader->bind();

        float L = drawData->DisplayPos.x;
        float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        float T = drawData->DisplayPos.y;
        float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        const float orthoProjection[4][4] =
        {
            { 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
            { 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
            { 0.0f,         0.0f,        -1.0f,   0.0f },
            { (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
        };
        m_shader->setUniformMat4("ProjMtx", &orthoProjection[0][0]);

        const auto clipOffset = drawData->DisplayPos;
        const auto clipScale = drawData->FramebufferScale;

        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const auto cmdList = drawData->CmdLists[n];
            const GLsizeiptr vertexBufferSize = static_cast<GLsizeiptr>(cmdList->VtxBuffer.Size) * static_cast<int>(sizeof(ImDrawVert));
            const GLsizeiptr indexBufferSize = static_cast<GLsizeiptr>(cmdList->IdxBuffer.Size) * static_cast<int>(sizeof(ImDrawIdx));
            glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, cmdList->VtxBuffer.Data, GL_STREAM_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, cmdList->IdxBuffer.Data, GL_STREAM_DRAW);
            for (int cmdIndex = 0; cmdIndex < cmdList->CmdBuffer.Size; cmdIndex++)
            {
                const auto cmd = &cmdList->CmdBuffer[cmdIndex];
                if (cmd->UserCallback != nullptr)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    /*if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                        ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
                    else
                        pcmd->UserCallback(cmd_list, pcmd);*/
                    throw std::runtime_error("Not implemented");
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    const ImVec2 clipMin((cmd->ClipRect.x - clipOffset.x) * clipScale.x,
                        (cmd->ClipRect.y - clipOffset.y) * clipScale.y);
                    const ImVec2 clipMax((cmd->ClipRect.z - clipOffset.x) * clipScale.x,
                        (cmd->ClipRect.w - clipOffset.y) * clipScale.y);
                    if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                        continue;

                    // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                    glScissor(static_cast<int>(clipMin.x), static_cast<int>(io.DisplaySize.y - clipMax.y),
                        static_cast<int>(clipMax.x - clipMin.x), static_cast<int>(clipMax.y - clipMin.y));

                    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(reinterpret_cast<intptr_t>(cmd->GetTexID())));
                    glDrawElements(GL_TRIANGLES,
                        static_cast<GLsizei>(cmd->ElemCount),
                        sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                        reinterpret_cast<void*>(static_cast<intptr_t>(cmd->IdxOffset * sizeof(ImDrawIdx))));
                }
            }
        }
    }

    void Gui::setCursorPos(const float x, const float y)
    {
        ImGui::GetIO().MousePos = ImVec2{ x, y };
    }

    void Gui::setMouseButton(const int button, const bool down)
    {
        ImGui::GetIO().MouseDown[button] = down;
    }
}
