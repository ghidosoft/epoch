#ifndef SRC_FRONTEND_WINDOW_H_
#define SRC_FRONTEND_WINDOW_H_

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

    private:
        GLFWwindow* m_window{};
    };
}

#endif
