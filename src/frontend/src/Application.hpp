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

#ifndef SRC_EPOCH_FRONTEND_APPLICATION_HPP_
#define SRC_EPOCH_FRONTEND_APPLICATION_HPP_

#include "ConfigurableShader.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace epoch
{
    class Emulator;
}

namespace epoch::frontend
{
    class AudioPlayer;
    class GraphicContext;
    class Gui;
    class SettingsManager;
    class Window;

    struct EmulatorEntry
    {
        std::string key;
        std::string name;
        std::function<std::shared_ptr<Emulator>()> factory;
    };
    struct ApplicationConfiguration
    {
        std::vector<EmulatorEntry> emulators;
    };

    class Application final
    {
    public:
        explicit Application(ApplicationConfiguration configuration);
        ~Application();

    public:
        Application(const Application& other) = delete;
        Application(Application&& other) noexcept = delete;
        Application& operator=(const Application& other) = delete;
        Application& operator=(Application&& other) noexcept = delete;

    public:
        int run();

        static constexpr auto AudioSampleRate = 48000;

    private:
        void init();

        void render();
        void renderGui();

        void setEmulatorEntry(const EmulatorEntry& entry);
        void setShader(std::size_t index);

        [[nodiscard]] std::string generateFileDialogFilters(bool save) const;

    private:
        std::unique_ptr<SettingsManager> m_settings{};
        const ApplicationConfiguration m_configuration{};
        std::shared_ptr<Emulator> m_emulator{};
        const EmulatorEntry* m_currentEntry{};

        double m_time{};
        double m_deltaTime{};

        std::unique_ptr<Window> m_window{};
        std::unique_ptr<GraphicContext> m_context{};
        std::unique_ptr<Gui> m_gui{};
        std::unique_ptr<AudioPlayer> m_audio{};
        std::vector<float> m_audioBuffer{};

        bool m_running{true};
        bool m_keepAspectRatio{true};
        bool m_fullscreen{false};
        bool m_showShaderSettings{false};

        std::vector<ConfigurableShader> m_shaders{};
        std::size_t m_shader{};

        struct profiling_t
        {
            static constexpr int COUNT = 64;
            int index{};
            float emulation[COUNT];
            float render[COUNT];
        } m_profiling{};
    };
}  // namespace epoch::frontend

#endif
