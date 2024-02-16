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

#ifndef SRC_FRONTEND_WINDOW_HPP_
#define SRC_FRONTEND_WINDOW_HPP_

#include <functional>
#include <string>

#include <epoch/core.hpp>

struct GLFWwindow;

namespace epoch::frontend
{
    enum class WindowMode { windowed, borderless, fullscreen };

    struct WindowInfo
    {
        std::string name;
        unsigned width;
        unsigned height;
        WindowMode mode{ WindowMode::windowed };
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

        [[nodiscard]] unsigned width() const { return m_width; }
        [[nodiscard]] unsigned height() const { return m_height; }
        [[nodiscard]] unsigned framebufferWidth() const { return m_framebufferWidth; }
        [[nodiscard]] unsigned framebufferHeight() const { return m_framebufferHeight; }

        [[nodiscard]] double time() const;

        void close() const;

        void resize(unsigned width, unsigned height) const;
        void mode(WindowMode mode);

        using CharCallback = std::function<void(unsigned int)>;
        using CursorEnterCallback = std::function<void(bool)>;
        using CursorPosCallback = std::function<void(float x, float y)>;
        using FileDropCallback = std::function<void(const char*)>;
        using FocusCallback = std::function<void(bool)>;
        using KeyboardCallback = std::function<void(Key key, KeyAction action)>;
        using MouseButtonCallback = std::function<void(int button, int action)>;
        using MouseWheelCallback = std::function<void(float x, float y)>;
        void setCharCallback(CharCallback callback);
        void setCursorEnterCallback(CursorEnterCallback callback);
        void setCursorPosCallback(CursorPosCallback callback);
        void setFileDropCallback(FileDropCallback callback);
        void setFocusCallback(FocusCallback callback);
        void setKeyboardCallback(KeyboardCallback callback);
        void setMouseButtonCallback(MouseButtonCallback callback);
        void setMouseWheelCallback(MouseWheelCallback callback);

    private:
        GLFWwindow* m_window{};

        unsigned m_width{}, m_height{};
        unsigned m_framebufferWidth{}, m_framebufferHeight{};
        WindowMode m_mode{ WindowMode::windowed };

        int m_lastX{}, m_lastY{};
        unsigned m_lastWidth{}, m_lastHeight{};

        CharCallback m_charCallback{};
        CursorEnterCallback m_cursorEnterCallback{};
        CursorPosCallback m_cursorPosCallback{};
        FileDropCallback m_fileDropCallback{};
        FocusCallback m_focusCallback{};
        KeyboardCallback m_keyboardCallback{};
        MouseButtonCallback m_mouseButtonCallback{};
        MouseWheelCallback m_mouseWheelCallback{};

    private:
        static void s_charCallback(GLFWwindow* glfwWindow, unsigned int c);
        static void s_cursorEnterCallback(GLFWwindow* glfwWindow, int entered);
        static void s_cursorPosCallback(GLFWwindow* glfwWindow, double x, double y);
        static void s_dropCallback(GLFWwindow* glfwWindow, int count, const char** paths);
        static void s_focusCallback(GLFWwindow* glfwWindow, int focused);
        static void s_framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height);
        static void s_resizeCallback(GLFWwindow* glfwWindow, int width, int height);
        static void s_keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);
        static void s_mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods);
        static void s_scrollCallback(GLFWwindow* glfwWindow, double x, double y);
    };
}

#endif
