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

#include "Window.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

static void s_glfwErrorCallback(const int code, const char* description)
{
    std::cerr << "GLFW error #" << code << ": " << description << std::endl;
}

namespace epoch::frontend
{
    Window::Window(const WindowInfo& info)
    {
        glfwSetErrorCallback(s_glfwErrorCallback);
        if (!glfwInit())
        {
            throw std::runtime_error("Cannot initialize GLFW.");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);

        m_window = glfwCreateWindow(static_cast<int>(info.width), static_cast<int>(info.height),
            info.title.c_str(), nullptr, nullptr);
        if (m_window == nullptr)
        {
            throw std::runtime_error("Cannot create GLFW window.");
        }

        int w, h;
        glfwGetWindowSize(m_window, &w, &h);
        m_width = static_cast<unsigned>(w);
        m_height = static_cast<unsigned>(h);
        glfwGetFramebufferSize(m_window, &w, &h);
        m_framebufferWidth = static_cast<unsigned>(w);
        m_framebufferHeight = static_cast<unsigned>(h);

        glfwSetWindowUserPointer(m_window, this);

        glfwSetCharCallback(m_window, s_charCallback);
        glfwSetCursorEnterCallback(m_window, s_cursorEnterCallback);
        glfwSetCursorPosCallback(m_window, s_cursorPosCallback);
        glfwSetDropCallback(m_window, s_dropCallback);
        glfwSetFramebufferSizeCallback(m_window, s_framebufferResizeCallback);
        glfwSetWindowSizeCallback(m_window, s_resizeCallback);
        glfwSetKeyCallback(m_window, s_keyCallback);
        glfwSetMouseButtonCallback(m_window, s_mouseButtonCallback);
        glfwSetScrollCallback(m_window, s_scrollCallback);
        glfwSetWindowFocusCallback(m_window, s_focusCallback);
        // TODO: setup other callbacks: monitor

        glfwMakeContextCurrent(m_window);

        glfwSwapInterval(1);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            throw std::runtime_error("Cannot initialize GLAD.");
        }

        mode(info.mode);
    }

    Window::~Window()
    {
        if (m_window != nullptr)
        {
            glfwDestroyWindow(m_window);
        }
        glfwTerminate();
    }

    bool Window::nextFrame() const
    {
        const auto result = !glfwWindowShouldClose(m_window);
        if (result)
        {
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }
        return result;
    }

    double Window::time() const
    {
        return glfwGetTime();
    }

    void Window::close() const
    {
        glfwSetWindowShouldClose(m_window, true);
    }

    void Window::resize(const unsigned width, const unsigned height) const
    {
        glfwSetWindowSize(m_window, static_cast<int>(width), static_cast<int>(height));
    }

    void Window::mode(const WindowMode mode)
    {
        if (mode != m_mode)
        {
            if (m_mode == WindowMode::windowed)
            {
                glfwGetWindowPos(m_window, &m_lastX, &m_lastY);
                m_lastWidth = m_width;
                m_lastHeight = m_height;
            }
            switch (mode)
            {
            case WindowMode::borderless:
                {
                    const auto monitor = glfwGetPrimaryMonitor();
                    const auto vidmode = glfwGetVideoMode(monitor);
                    glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_FALSE);
                    glfwSetWindowPos(m_window, 0, 0);
                    glfwSetWindowSize(m_window, vidmode->width, vidmode->height);
                }
                break;
            case WindowMode::fullscreen:
                {
                    const auto monitor = glfwGetPrimaryMonitor();
                    const auto vidmode = glfwGetVideoMode(monitor);
                    glfwSetWindowMonitor(m_window, monitor, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
                }
                break;
            case WindowMode::windowed:
                glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_TRUE);
                glfwSetWindowMonitor(m_window, nullptr, m_lastX, m_lastY, static_cast<int>(m_lastWidth), static_cast<int>(m_lastHeight), GLFW_DONT_CARE);
                break;
            }
            m_mode = mode;
        }
    }

    void Window::setTitle(const std::string& title) const
    {
        glfwSetWindowTitle(m_window, title.c_str());
    }

    void Window::setCharCallback(CharCallback callback)
    {
        m_charCallback = std::move(callback);
    }

    void Window::setCursorEnterCallback(CursorEnterCallback callback)
    {
        m_cursorEnterCallback = std::move(callback);
    }

    void Window::setCursorPosCallback(CursorPosCallback callback)
    {
        m_cursorPosCallback = std::move(callback);
    }

    void Window::setFileDropCallback(FileDropCallback callback)
    {
        m_fileDropCallback = std::move(callback);
    }

    void Window::setFocusCallback(FocusCallback callback)
    {
        m_focusCallback = std::move(callback);
    }

    void Window::setKeyboardCallback(KeyboardCallback callback)
    {
        m_keyboardCallback = std::move(callback);
    }

    void Window::setMouseButtonCallback(MouseButtonCallback callback)
    {
        m_mouseButtonCallback = std::move(callback);
    }

    void Window::setMouseWheelCallback(MouseWheelCallback callback)
    {
        m_mouseWheelCallback = std::move(callback);
    }

    void Window::s_charCallback(GLFWwindow* glfwWindow, const unsigned int c)
    {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        if (window->m_charCallback)
        {
            window->m_charCallback(c);
        }
    }

    void Window::s_cursorEnterCallback(GLFWwindow* glfwWindow, const int entered)
    {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        if (window->m_cursorEnterCallback)
        {
            window->m_cursorEnterCallback(entered);
        }
    }

    void Window::s_cursorPosCallback(GLFWwindow* glfwWindow, const double x, const double y)
    {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        if (window->m_cursorPosCallback)
        {
            window->m_cursorPosCallback(static_cast<float>(x), static_cast<float>(y));
        }
    }

    void Window::s_dropCallback(GLFWwindow *glfwWindow, int count, const char** paths)
    {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        if (window->m_fileDropCallback && count >= 1)
        {
            window->m_fileDropCallback(paths[0]);
        }
    }

    void Window::s_focusCallback(GLFWwindow* glfwWindow, const int focused)
    {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        if (window->m_focusCallback)
        {
            window->m_focusCallback(focused);
        }
    }

    void Window::s_framebufferResizeCallback(GLFWwindow* glfwWindow, const int width, const int height)
    {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        window->m_framebufferWidth = static_cast<unsigned>(width);
        window->m_framebufferHeight = static_cast<unsigned>(height);
    }

    void Window::s_resizeCallback(GLFWwindow* glfwWindow, const int width, const int height)
    {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        window->m_width = static_cast<unsigned>(width);
        window->m_height = static_cast<unsigned>(height);
    }

    void Window::s_keyCallback(GLFWwindow* glfwWindow, const int key, int scancode, const int action, int mods)
    {
        if (key != GLFW_KEY_UNKNOWN)
        {
            const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
            if (window->m_keyboardCallback)
            {
                auto keyAction = KeyAction::press;
                if (action == GLFW_RELEASE)
                    keyAction = KeyAction::release;
                else if (action == GLFW_REPEAT)
                    keyAction = KeyAction::repeat;
                window->m_keyboardCallback(static_cast<Key>(key), keyAction);
            }
        }
    }

    void Window::s_mouseButtonCallback(GLFWwindow* glfwWindow, const int button, const int action, const int mods)
    {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        if (window->m_mouseButtonCallback)
        {
            window->m_mouseButtonCallback(button, action);
        }
    }

    void Window::s_scrollCallback(GLFWwindow* glfwWindow, const double x, const double y)
    {
        const auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
        if (window->m_mouseWheelCallback)
        {
            window->m_mouseWheelCallback(static_cast<float>(x), static_cast<float>(y));
        }
    }
}
