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

#include "Window.h"

#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace epoch::frontend
{
    Window::Window(const WindowInfo& info)
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Cannot initialize GLFW.");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);

        m_window = glfwCreateWindow(info.width * 3, info.height * 3, info.name.c_str(), nullptr, nullptr);
        if (m_window == nullptr)
        {
            throw std::runtime_error("Cannot create GLFW window.");
        }

        glfwSetWindowUserPointer(m_window, this);

        // TODO: setup callbacks

        glfwMakeContextCurrent(m_window);

        glfwSwapInterval(1);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            throw std::runtime_error("Cannot initialize GLAD.");
        }
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
}
