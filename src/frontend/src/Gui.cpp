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

#include <epoch/core.hpp>

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
    static ImGuiKey mapKeyToImGui(const Key key)
    {
        switch (key)
        {
        case Key::Tab: return ImGuiKey_Tab;
        case Key::Left: return ImGuiKey_LeftArrow;
        case Key::Right: return ImGuiKey_RightArrow;
        case Key::Up: return ImGuiKey_UpArrow;
        case Key::Down: return ImGuiKey_DownArrow;
        case Key::PageUp: return ImGuiKey_PageUp;
        case Key::PageDown: return ImGuiKey_PageDown;
        case Key::Home: return ImGuiKey_Home;
        case Key::End: return ImGuiKey_End;
        case Key::Insert: return ImGuiKey_Insert;
        case Key::Delete: return ImGuiKey_Delete;
        case Key::Backspace: return ImGuiKey_Backspace;
        case Key::Space: return ImGuiKey_Space;
        case Key::Enter: return ImGuiKey_Enter;
        case Key::Escape: return ImGuiKey_Escape;
        case Key::Apostrophe: return ImGuiKey_Apostrophe;
        case Key::Comma: return ImGuiKey_Comma;
        case Key::Minus: return ImGuiKey_Minus;
        case Key::Period: return ImGuiKey_Period;
        case Key::Slash: return ImGuiKey_Slash;
        case Key::Semicolon: return ImGuiKey_Semicolon;
        case Key::Equal: return ImGuiKey_Equal;
        case Key::LeftBracket: return ImGuiKey_LeftBracket;
        case Key::Backslash: return ImGuiKey_Backslash;
        case Key::RightBracket: return ImGuiKey_RightBracket;
        case Key::GraveAccent: return ImGuiKey_GraveAccent;
        case Key::CapsLock: return ImGuiKey_CapsLock;
        case Key::ScrollLock: return ImGuiKey_ScrollLock;
        case Key::NumLock: return ImGuiKey_NumLock;
        case Key::PrintScreen: return ImGuiKey_PrintScreen;
        case Key::Pause: return ImGuiKey_Pause;
        case Key::KeyPad0: return ImGuiKey_Keypad0;
        case Key::KeyPad1: return ImGuiKey_Keypad1;
        case Key::KeyPad2: return ImGuiKey_Keypad2;
        case Key::KeyPad3: return ImGuiKey_Keypad3;
        case Key::KeyPad4: return ImGuiKey_Keypad4;
        case Key::KeyPad5: return ImGuiKey_Keypad5;
        case Key::KeyPad6: return ImGuiKey_Keypad6;
        case Key::KeyPad7: return ImGuiKey_Keypad7;
        case Key::KeyPad8: return ImGuiKey_Keypad8;
        case Key::KeyPad9: return ImGuiKey_Keypad9;
        case Key::KeyPadDecimal: return ImGuiKey_KeypadDecimal;
        case Key::KeyPadDivide: return ImGuiKey_KeypadDivide;
        case Key::KeyPadMultiply: return ImGuiKey_KeypadMultiply;
        case Key::KeyPadSubtract: return ImGuiKey_KeypadSubtract;
        case Key::KeyPadAdd: return ImGuiKey_KeypadAdd;
        case Key::KeyPadEnter: return ImGuiKey_KeypadEnter;
        case Key::KeyPadEqual: return ImGuiKey_KeypadEqual;
        case Key::LeftShift: return ImGuiKey_LeftShift;
        case Key::LeftControl: return ImGuiKey_LeftCtrl;
        case Key::LeftAlt: return ImGuiKey_LeftAlt;
        case Key::LeftSuper: return ImGuiKey_LeftSuper;
        case Key::RightShift: return ImGuiKey_RightShift;
        case Key::RightControl: return ImGuiKey_RightCtrl;
        case Key::RightAlt: return ImGuiKey_RightAlt;
        case Key::RightSuper: return ImGuiKey_RightSuper;
        case Key::Menu: return ImGuiKey_Menu;
        case Key::D0: return ImGuiKey_0;
        case Key::D1: return ImGuiKey_1;
        case Key::D2: return ImGuiKey_2;
        case Key::D3: return ImGuiKey_3;
        case Key::D4: return ImGuiKey_4;
        case Key::D5: return ImGuiKey_5;
        case Key::D6: return ImGuiKey_6;
        case Key::D7: return ImGuiKey_7;
        case Key::D8: return ImGuiKey_8;
        case Key::D9: return ImGuiKey_9;
        case Key::A: return ImGuiKey_A;
        case Key::B: return ImGuiKey_B;
        case Key::C: return ImGuiKey_C;
        case Key::D: return ImGuiKey_D;
        case Key::E: return ImGuiKey_E;
        case Key::F: return ImGuiKey_F;
        case Key::G: return ImGuiKey_G;
        case Key::H: return ImGuiKey_H;
        case Key::I: return ImGuiKey_I;
        case Key::J: return ImGuiKey_J;
        case Key::K: return ImGuiKey_K;
        case Key::L: return ImGuiKey_L;
        case Key::M: return ImGuiKey_M;
        case Key::N: return ImGuiKey_N;
        case Key::O: return ImGuiKey_O;
        case Key::P: return ImGuiKey_P;
        case Key::Q: return ImGuiKey_Q;
        case Key::R: return ImGuiKey_R;
        case Key::S: return ImGuiKey_S;
        case Key::T: return ImGuiKey_T;
        case Key::U: return ImGuiKey_U;
        case Key::V: return ImGuiKey_V;
        case Key::W: return ImGuiKey_W;
        case Key::X: return ImGuiKey_X;
        case Key::Y: return ImGuiKey_Y;
        case Key::Z: return ImGuiKey_Z;
        case Key::F1: return ImGuiKey_F1;
        case Key::F2: return ImGuiKey_F2;
        case Key::F3: return ImGuiKey_F3;
        case Key::F4: return ImGuiKey_F4;
        case Key::F5: return ImGuiKey_F5;
        case Key::F6: return ImGuiKey_F6;
        case Key::F7: return ImGuiKey_F7;
        case Key::F8: return ImGuiKey_F8;
        case Key::F9: return ImGuiKey_F9;
        case Key::F10: return ImGuiKey_F10;
        case Key::F11: return ImGuiKey_F11;
        case Key::F12: return ImGuiKey_F12;
        default: return ImGuiKey_None;
        }
    }

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

    void Gui::newFrame(const int width, const int height, const int framebufferWidth, const int framebufferHeight, const double deltaTime) const
    {
        ImGui::GetIO().DisplaySize = ImVec2{ static_cast<float>(framebufferWidth), static_cast<float>(framebufferHeight) };
        if (width > 0 && height > 0)
        {
            ImGui::GetIO().DisplayFramebufferScale = ImVec2{
                static_cast<float>(framebufferWidth) / static_cast<float>(width),
                static_cast<float>(framebufferHeight) / static_cast<float>(height)
            };
        }
        ImGui::GetIO().DeltaTime = static_cast<float>(deltaTime);
        ImGui::NewFrame();
    }

    void Gui::render() const
    {
        ImGui::Render();
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

        const float L = drawData->DisplayPos.x;
        const float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        const float T = drawData->DisplayPos.y;
        const float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        const float orthoProjection[4][4] =
        {
            { 2.0f / (R - L),       0.0f,               0.0f,   0.0f },
            { 0.0f,                 2.0f / (T - B),     0.0f,   0.0f },
            { 0.0f,                 0.0f,               -1.0f,  0.0f },
            { (R + L) / (L - R),    (T + B) / (B - T),  0.0f,   1.0f },
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
                    glScissor(static_cast<int>(clipMin.x), static_cast<int>(ImGui::GetIO().DisplaySize.y - clipMax.y),
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

    void Gui::setCursorPos(const float x, const float y) const
    {
        ImGui::GetIO().MousePos = ImVec2{ x, y };
    }

    void Gui::charEvent(const unsigned int c) const
    {
        ImGui::GetIO().AddInputCharacter(c);
    }

    void Gui::focusEvent(const bool focused) const
    {
        ImGui::GetIO().AddFocusEvent(focused);
    }

    void Gui::keyEvent(const Key key, const KeyAction action) const
    {
        if (action != KeyAction::press && action != KeyAction::release) return;

        const auto imguiKey = mapKeyToImGui(key);
        ImGui::GetIO().AddKeyEvent(imguiKey, action == KeyAction::press);
    }

    void Gui::mouseWheelEvent(const float x, const float y) const
    {
        ImGui::GetIO().AddMouseWheelEvent(x, y);
    }

    void Gui::setMouseButton(const int button, const bool down) const
    {
        ImGui::GetIO().MouseDown[button] = down;
    }

    bool Gui::wantKeyboardEvents() const
    {
        return ImGui::GetIO().WantCaptureKeyboard;
    }
}
