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
#include <imgui.h>

#include "GraphicContext.h"
#include "Gui.h"
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
        m_gui = std::make_unique<Gui>();

        m_window->setCursorPosCallback(
            [&](const float x, const float y){ m_gui->setCursorPos(x, y); });
        m_window->setKeyboardCallback(
            [&](const Key key, const KeyAction action) { m_emulator->keyEvent(key, action); });
        m_window->setMouseButtonCallback(
            [&](const int button, const int action) { m_gui->setMouseButton(button, action == 1); });
    }

    Application::~Application() = default;

    int Application::run()
    {
        m_context->init(m_emulator->info().width, m_emulator->info().height);
        m_emulator->reset();
        while (m_window->nextFrame())
        {
            if (m_running)
            {
                m_emulator->frame();
            }
            m_context->updateScreen(m_emulator->screenBuffer());
            render();
        }
        return 0;
    }

    void Application::render()
    {
        m_context->viewport(m_window->width(), m_window->height());
        m_context->renderScreen();

        m_gui->newFrame(m_window->width(), m_window->height());
        renderGui();
        m_gui->render();
    }

    void Application::renderGui()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Restore snapshot")) { m_emulator->loadSnapshot("test.sna"); }
                if (ImGui::MenuItem("Take snapshot")) { m_emulator->saveSnapshot("test.sna"); }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { m_window->close(); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Emulator"))
            {
                if (ImGui::MenuItem("Run", nullptr, &m_running)) {}
                if (ImGui::MenuItem("Reset")) { m_emulator->reset(); }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}
