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

#include <memory>
#include <vector>

#include "ConfigurableShader.hpp"

namespace epoch
{
    class Emulator;
}

namespace epoch::frontend
{
    class AudioPlayer;
    class GraphicContext;
    class Gui;
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

        static constexpr auto AudioSampleRate = 48000;
        static constexpr auto AudioChannels = 1;

    private:
        void render();
        void renderGui();

        std::string generateFileDialogFilters(bool save) const;

    private:
        std::shared_ptr<Emulator> m_emulator{};

        double m_time{};
        double m_deltaTime{};

        std::unique_ptr<Window> m_window{};
        std::unique_ptr<GraphicContext> m_context{};
        std::unique_ptr<Gui> m_gui{};
        std::unique_ptr<AudioPlayer> m_audio{};
        std::vector<float> m_audioBuffer{};

        bool m_running{ true };
        bool m_keepAspectRatio{ true };
        bool m_fullscreen{ false };
        bool m_showShaderSettings{ false };

        std::vector<ConfigurableShader> m_shaders{};
        std::size_t m_shader{};
    };
}

#endif
