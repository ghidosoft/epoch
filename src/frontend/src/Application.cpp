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

#include "Application.hpp"

#include "Audio.hpp"
#include "GraphicContext.hpp"
#include "Gui.hpp"
#include "SettingsManager.hpp"
#include "Shaders.hpp"
#include "Window.hpp"

#include <epoch/core.hpp>

#include <imgui.h>

#include <ImGuiFileDialog.h>

#include <sstream>

namespace epoch::frontend
{
    Application::Application(ApplicationConfiguration configuration)
        : m_settings{std::make_unique<SettingsManager>()}, m_configuration{std::move(configuration)}
    {
        assert(!m_configuration.emulators.empty());
        m_settings->load();
        for (auto& entry : m_configuration.emulators)
        {
            if (entry.key == m_settings->current().emulator.key)
            {
                m_currentEntry = &entry;
                break;
            }
        }
        if (!m_currentEntry)
        {
            const auto& entry = m_configuration.emulators[0];
            m_currentEntry = &entry;
        }
        m_emulator = m_currentEntry->factory();

        m_shaders.emplace_back("default", "<none>", shaders::DEFAULT);
        m_shaders.emplace_back("crt-easymode", "crt-easymode", shaders::CRT_EASYMODE);
        m_shaders.emplace_back("crt-geom", "crt-geom", shaders::CRT_GEOM);

        init();
    }

    Application::~Application() = default;

    int Application::run()
    {
        m_time = m_window->time();
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
            auto currentTime = m_window->time();
            if (currentTime <= m_time) currentTime = m_time + 0.00001;
            m_deltaTime = currentTime - m_time;
            m_time = currentTime;
            m_context->updateScreen(m_emulator->screenBuffer());
            render();
        }
        m_settings->current().ui.imgui = m_gui->generateSettings();
        if (m_settings->dirty())
        {
            m_settings->save();
        }
        return 0;
    }

    void Application::init()
    {
        assert(!m_window);
        const auto& emulatorInfo = m_emulator->info();
        m_window = std::make_unique<Window>(WindowInfo{
            .title = "Epoch emulator: " + emulatorInfo.name,
            .width = emulatorInfo.width * 2,
            .height = emulatorInfo.height * 2,
        });
        m_context = std::make_unique<GraphicContext>();
        m_gui = std::make_unique<Gui>(m_settings->current().ui.imgui.c_str());
        m_audio = std::make_unique<AudioPlayer>(AudioSampleRate, AudioChannels);

        m_window->setCharCallback([&](const unsigned int c) { m_gui->charEvent(c); });
        m_window->setCursorEnterCallback([&](const bool entered) { m_gui->cursorEnterEvent(entered); });
        m_window->setCursorPosCallback([&](const float x, const float y) { m_gui->cursorPosEvent(x, y); });
        m_window->setFileDropCallback([&](const char* path) { m_emulator->load(path); });
        m_window->setFocusCallback([&](const bool focused) { m_gui->focusEvent(focused); });
        m_window->setKeyboardCallback(
            [&](const Key key, const KeyAction action)
            {
                if (!m_gui->wantKeyboardEvents()) m_emulator->keyEvent(key, action);
                m_gui->keyEvent(key, action);
            });
        m_window->setMouseButtonCallback([&](const int button, const int action)
                                         { m_gui->mouseButtonEvent(button, action == 1); });
        m_window->setMouseWheelCallback([&](const float x, const float y) { m_gui->mouseWheelEvent(x, y); });

        m_context->init(m_emulator->info().width, m_emulator->info().height);

        std::size_t shader = 0;
        for (std::size_t i = 0; i < m_shaders.size(); ++i)
        {
            if (m_shaders[i].key() == m_settings->current().ui.shader)
            {
                shader = i;
                break;
            }
        }
        setShader(shader);

        m_emulator->reset();
    }

    void Application::render()
    {
        if (m_keepAspectRatio)
        {
            const auto emulatorAspectRatio = m_emulator->info().aspectRatio();
            const auto windowAspectRatio =
                static_cast<float>(m_window->framebufferWidth()) / static_cast<float>(m_window->framebufferHeight());
            auto x = 0u, y = 0u;
            auto width = m_window->framebufferWidth();
            auto height = m_window->framebufferHeight();
            if (emulatorAspectRatio > windowAspectRatio)
            {
                height = static_cast<int>(static_cast<float>(width) / emulatorAspectRatio);
                y = (m_window->framebufferHeight() - height) / 2;
            }
            else
            {
                width = static_cast<int>(static_cast<float>(height) * emulatorAspectRatio);
                x = (m_window->framebufferWidth() - width) / 2;
            }
            m_context->viewport(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width),
                                static_cast<int>(height));
        }
        else
        {
            m_context->viewport(0, 0, static_cast<int>(m_window->framebufferWidth()),
                                static_cast<int>(m_window->framebufferHeight()));
        }
        m_context->renderScreen();

        m_context->viewport(0, 0, static_cast<int>(m_window->framebufferWidth()),
                            static_cast<int>(m_window->framebufferHeight()));
        m_gui->newFrame(m_window->width(), m_window->height(), m_window->framebufferWidth(),
                        m_window->framebufferHeight(), m_deltaTime);
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
                    const std::string filters = generateFileDialogFilters(false);
                    const IGFD::FileDialogConfig config{
                        .path = m_settings->current().ui.lastLoadPath,
                        .flags = ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_Modal |
                                 ImGuiFileDialogFlags_DisableCreateDirectoryButton,
                    };
                    ImGuiFileDialog::Instance()->OpenDialog("LoadDialogKey", "Load", filters.c_str(), config);
                }
                if (ImGui::MenuItem("Save"))
                {
                    const std::string filters = generateFileDialogFilters(true);
                    const IGFD::FileDialogConfig config{
                        .path = m_settings->current().ui.lastSavePath,
                        .flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal,
                    };
                    ImGuiFileDialog::Instance()->OpenDialog("SaveDialogKey", "Save", filters.c_str(), config);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit"))
                {
                    m_window->close();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Emulator"))
            {
                if (ImGui::MenuItem("Run", nullptr, &m_running))
                {
                }
                if (ImGui::MenuItem("Reset"))
                {
                    m_emulator->reset();
                }
                if (!m_configuration.emulators.empty())
                {
                    ImGui::Separator();
                    for (const auto& entry : m_configuration.emulators)
                    {
                        if (ImGui::MenuItem(entry.name.c_str(), nullptr, m_currentEntry == &entry))
                        {
                            setEmulatorEntry(entry);
                        }
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window"))
            {
                if (ImGui::MenuItem("Keep aspect ratio", nullptr, &m_keepAspectRatio))
                {
                }
                ImGui::Separator();
                if (ImGui::MenuItem("1X size"))
                {
                    m_window->resize(m_emulator->info().width, m_emulator->info().height);
                }
                if (ImGui::MenuItem("2X size"))
                {
                    m_window->resize(m_emulator->info().width * 2, m_emulator->info().height * 2);
                }
                if (ImGui::MenuItem("3X size"))
                {
                    m_window->resize(m_emulator->info().width * 3, m_emulator->info().height * 3);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Fullscreen", nullptr, &m_fullscreen))
                {
                    m_window->mode(m_fullscreen ? WindowMode::borderless : WindowMode::windowed);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Shader settings", nullptr, &m_showShaderSettings))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (m_showShaderSettings)
        {
            ImGui::SetNextWindowSize(
                {
                    300,
                    450,
                },
                ImGuiCond_Once);
            if (ImGui::Begin("Shader settings", &m_showShaderSettings))
            {
                if (ImGui::BeginCombo("Shader", m_shaders[m_shader].name().c_str()))
                {
                    for (std::size_t i = 0; i < m_shaders.size(); i++)
                    {
                        if (ImGui::Selectable(m_shaders[i].name().c_str(), i == m_shader))
                        {
                            setShader(i);
                        }
                        if (i == m_shader) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (!m_shaders[m_shader].parameters().empty())
                {
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::PushItemWidth(ImGui::GetWindowSize().x / 4);
                    for (auto& parameter : m_shaders[m_shader].parameters())
                    {
                        if (ImGui::SliderFloat(parameter.description.c_str(), &parameter.value, parameter.min,
                                               parameter.max))
                        {
                            m_context->updateShaderParameters(m_shaders[m_shader]);
                        }
                    }
                    ImGui::PopItemWidth();
                    if (ImGui::Button("Reset values"))
                    {
                        for (auto& parameter : m_shaders[m_shader].parameters())
                        {
                            parameter.value = parameter.defaultValue;
                        }
                        m_context->updateShaderParameters(m_shaders[m_shader]);
                    }
                }
            }
            ImGui::End();
        }

        const ImVec2 screenSize{static_cast<float>(m_window->width()), static_cast<float>(m_window->height())};
        ImGui::SetNextWindowPos({0, 0});

        if (ImGuiFileDialog::Instance()->Display(
                "LoadDialogKey", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize,
                screenSize, screenSize))
        {
            m_settings->current().ui.lastLoadPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                m_emulator->load(filePathName);
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display(
                "SaveDialogKey", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize,
                screenSize, screenSize))
        {
            m_settings->current().ui.lastSavePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                const std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                m_emulator->save(filePathName);
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }

    void Application::setEmulatorEntry(const EmulatorEntry& entry)
    {
        m_emulator = entry.factory();
        m_currentEntry = &entry;
        m_settings->current().emulator.key = m_currentEntry->key;
        m_window->setTitle("Epoch emulator: " + m_emulator->info().name);
    }

    void Application::setShader(const std::size_t index)
    {
        m_shader = index;
        m_context->updateShader(m_shaders[m_shader]);
        m_settings->current().ui.shader = m_shaders[m_shader].key();
    }

    std::string Application::generateFileDialogFilters(const bool save) const
    {
        std::ostringstream ss;
        ss << "Supported files{";
        bool empty{true};
        for (const auto& format : m_emulator->info().fileFormats)
        {
            if (save ? format.save : format.load)
            {
                if (!empty) ss << ",";
                ss << format.extensions;
                empty = false;
            }
        }
        ss << "},";
        for (const auto& format : m_emulator->info().fileFormats)
        {
            if (save ? format.save : format.load)
            {
                ss << format.description;
                ss << "{" << format.extensions << "},";
            }
        }
        ss << "All files {.*}";
        return ss.str();
    }
}  // namespace epoch::frontend
