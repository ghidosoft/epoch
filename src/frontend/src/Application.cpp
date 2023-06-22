#include "Application.h"

#include <epoch/core.h>

#include "GraphicContext.h"
#include "Window.h"

namespace epoch::frontend
{
    Application::Application(std::shared_ptr<Emulator> emulator) : m_emulator{ std::move(emulator) }
    {
        const auto& emulatorInfo = m_emulator->info();
        m_window = std::make_unique<Window>(WindowInfo{
            .name = "Epoch emulator: " + emulatorInfo.name,
            .width = emulatorInfo.width,
            .height = emulatorInfo.height,
        });
        m_context = std::make_unique<GraphicContext>();
    }

    Application::~Application()
    {
        m_context = nullptr;
        m_window = nullptr;
    }

    int Application::run()
    {
        m_context->init(m_emulator->info().width, m_emulator->info().height);
        m_emulator->reset();
        while (m_window->nextFrame())
        {
            // TODO: simulate an entire frame (or sync with audio?)
            m_emulator->clock();
            m_context->updateScreen(m_emulator->screenBuffer());
            render();
        }
        return 0;
    }

    void Application::render()
    {
        m_context->renderScreen();

        renderGui();
    }

    void Application::renderGui()
    {
    }
}
