#include "Application.h"

#include <epoch/core.h>

#include "Window.h"

namespace epoch
{
    Application::Application(std::shared_ptr<Emulator> emulator) : m_emulator{ std::move(emulator) }
    {
        m_window = std::make_unique<Window>(WindowInfo{
            .name = "Epoch - ZX Spectrum emulator"
        });
    }

    Application::~Application()
    {
        m_window = nullptr;
    }

    int Application::run()
    {
        m_emulator->reset();
        while (m_window->nextFrame())
        {
            // TODO: simulate an entire frame
            m_emulator->clock();
            // TODO: render
        }
        return 0;
    }
}
