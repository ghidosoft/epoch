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

#include "Application.hpp"

#include <epoch/core.hpp>
#include <imgui.h>

#include "Audio.hpp"
#include "GraphicContext.hpp"
#include "Gui.hpp"
#include "Platform.hpp"
#include "Window.hpp"

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
        m_platform = std::make_unique<Platform>();
        m_context = std::make_unique<GraphicContext>();
        m_gui = std::make_unique<Gui>();
        m_audio = std::make_unique<AudioPlayer>(AudioSampleRate, AudioChannels);

        m_window->setCursorPosCallback(
            [&](const float x, const float y) { m_gui->setCursorPos(x, y); });
        m_window->setFileDropCallback(
            [&](const char* path) { m_emulator->load(path); });
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
                const auto samples = m_audio->neededSamples();
                if (samples > 0)
                {
                    m_audioBuffer.resize(samples);
                    for (unsigned long i = 0; i < samples; i++)
                    {
                        m_audioBuffer[i] = m_emulator->generateNextAudioSample();
                    }
                    m_audio->push(m_audioBuffer);
                }
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
                if (ImGui::MenuItem("Load"))
                {
                    if (const auto path = m_platform->openDialog(); !path.empty())
                    {
                        m_emulator->load(path);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Restore test.sna")) { m_emulator->load("test.sna"); }
                if (ImGui::MenuItem("Take test.sna")) { m_emulator->save("test.sna"); }
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
            if (ImGui::BeginMenu("Window"))
            {
                if (ImGui::MenuItem("1X size")) { m_window->resize(m_emulator->info().width, m_emulator->info().height); }
                if (ImGui::MenuItem("2X size")) { m_window->resize(m_emulator->info().width * 2, m_emulator->info().height * 2); }
                if (ImGui::MenuItem("3X size")) { m_window->resize(m_emulator->info().width * 3, m_emulator->info().height * 3); }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}
