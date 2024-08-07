/* This file is part of Epoch, Copyright (C) 2024 Andrea Ghidini.
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

#ifndef SRC_FRONTEND_GUI_HPP_
#define SRC_FRONTEND_GUI_HPP_

#include <glad/glad.h>

#include <memory>

struct ImFont;
struct ImGuiContext;

namespace epoch
{
    enum class Key;
    enum class KeyAction;
}  // namespace epoch

namespace epoch::frontend
{
    class Shader;

    class Gui final
    {
    public:
        explicit Gui(const char* settings);
        ~Gui();

    public:
        Gui(const Gui& other) = delete;
        Gui(Gui&& other) noexcept = delete;
        Gui& operator=(const Gui& other) = delete;
        Gui& operator=(Gui&& other) noexcept = delete;

    public:
        void newFrame(unsigned width, unsigned height, unsigned framebufferWidth, unsigned framebufferHeight,
                      double deltaTime) const;
        void render() const;

        void contentScaleEvent(float scale) const;
        void cursorEnterEvent(bool entered);
        void cursorPosEvent(float x, float y);
        void charEvent(unsigned int c) const;
        void focusEvent(bool focused) const;
        void keyEvent(Key key, KeyAction action) const;
        void mouseButtonEvent(int button, bool down) const;
        void mouseWheelEvent(float x, float y) const;

        [[nodiscard]] bool wantKeyboardEvents() const;

        [[nodiscard]] const char* generateSettings() const;

    private:
        static constexpr float FontSize = 13.f;

    private:
        std::unique_ptr<Shader> m_shader{};
        GLuint m_vao{};
        GLuint m_vertexBuffer{}, m_indexBuffer{};
        GLuint m_fontTexture{};

        ImGuiContext* m_context{};
        ImFont* m_font{};

        float m_lastMouseX{}, m_lastMouseY{};

        void setupStyle(float scale) const;
    };
}  // namespace epoch::frontend

#endif
