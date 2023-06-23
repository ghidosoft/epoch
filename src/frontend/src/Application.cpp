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
