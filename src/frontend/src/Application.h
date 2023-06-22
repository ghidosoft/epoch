#ifndef SRC_EPOCH_FRONTEND_APPLICATION_H_
#define SRC_EPOCH_FRONTEND_APPLICATION_H_

#include <memory>

namespace epoch
{
    class Emulator;
}

namespace epoch::frontend
{
    class GraphicContext;
    class Window;

    class Application final
    {
    public:
        explicit Application(std::shared_ptr<Emulator> emulator);
        ~Application();

    public:
        Application(const Application& other) = delete;
        Application(Application&& other) noexcept = delete;
        Application& operator=(const Application& other) = delete;
        Application& operator=(Application&& other) noexcept = delete;

    public:
        int run();

    private:
        void render();
        void renderGui();

    private:
        std::shared_ptr<Emulator> m_emulator{};

        std::unique_ptr<Window> m_window{};
        std::unique_ptr<GraphicContext> m_context{};
    };
}

#endif
