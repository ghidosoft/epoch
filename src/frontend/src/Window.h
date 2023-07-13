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

#ifndef SRC_FRONTEND_WINDOW_H_
#define SRC_FRONTEND_WINDOW_H_

#include <functional>
#include <string>

struct GLFWwindow;

namespace epoch::frontend
{
    struct WindowInfo
    {
        std::string name;
        int width;
        int height;
    };

    class Window final
    {
    public:
        explicit Window(const WindowInfo& info);
        ~Window();

    public:
        Window(const Window& other) = delete;
        Window(Window&& other) noexcept = delete;
        Window& operator=(const Window& other) = delete;
        Window& operator=(Window&& other) noexcept = delete;

    public:
        [[nodiscard]] bool nextFrame() const;
        [[nodiscard]] int width() const { return m_width; }
        [[nodiscard]] int height() const { return m_height; }

        using CursorPosCallback = std::function<void(float x, float y)>;
        using MouseButtonCallback = std::function<void(int button, int action)>;
        void setCursorPosCallback(CursorPosCallback callback);
        void setMouseButtonCallback(MouseButtonCallback callback);

    private:
        GLFWwindow* m_window{};

        int m_width{}, m_height{};

        CursorPosCallback m_cursorPosCallback{};
        MouseButtonCallback m_mouseButtonCallback{};

    private:
        static void s_framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height);
        static void s_keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);
        static void s_cursorPosCallback(GLFWwindow* glfwWindow, double x, double y);
        static void s_mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods);
    };
}

#endif
