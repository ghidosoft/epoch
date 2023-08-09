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

#ifndef SRC_FRONTEND_EMULATIONCONTROLLER_H_
#define SRC_FRONTEND_EMULATIONCONTROLLER_H_

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "CircularBuffer.h"

namespace epoch
{
    class Emulator;
}

namespace epoch::frontend
{
    class ClockTimer final
    {
    public:
        ClockTimer()
        {
            m_last = std::chrono::high_resolution_clock::now();
        }

        void tick()
        {
            const auto now = std::chrono::high_resolution_clock::now();
            const auto elapsed = std::chrono::duration<double, std::ratio<1>>(now - m_last).count();
            m_last = now;
            m_ticks++;
            m_elapsed += elapsed;
            if (m_ticks > 10)
            {
                m_tickDuration = m_elapsed / static_cast<double>(m_ticks);
                m_ticksPerSecond = static_cast<double>(m_ticks) / m_elapsed;
                std::cout << "Tick duration " << m_tickDuration << " (" << m_ticksPerSecond << " ticks/s)\n";
                m_elapsed = 0;
                m_ticks = 0;
            }
        }

        [[nodiscard]] double tickDuration() const { return m_elapsed / static_cast<double>(m_ticks); }

    private:
        double m_elapsed{};
        int m_ticks{};

        double m_tickDuration{};
        double m_ticksPerSecond{};

        std::chrono::time_point<std::chrono::high_resolution_clock> m_last{};
    };

    class EmulationController final
    {
    public:
        explicit EmulationController(std::shared_ptr<Emulator> emulator);
        ~EmulationController();

    public:
        EmulationController(const EmulationController& other) = delete;
        EmulationController(EmulationController&& other) noexcept = delete;
        EmulationController& operator=(const EmulationController& other) = delete;
        EmulationController& operator=(EmulationController&& other) noexcept = delete;

    public:
        void start();
        void suspend();
        void resume();
        void setSpeed(double speed);

    private:
        void run();

    private:
        std::shared_ptr<Emulator> m_emulator;
        bool m_shouldExit{};

        bool m_paused{};

        std::thread m_thread{};
        std::mutex m_mutex{};
        std::condition_variable m_conditionVariable;

        CircularBuffer<float, 16384> m_audioBuffer{};
        double m_speed{ 1. };

        ClockTimer m_timer{};

        const double m_frameDuration;
    };
}

#endif
