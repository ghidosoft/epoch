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
