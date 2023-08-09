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

#include "EmulationController.h"

#include <cassert>
#include <chrono>

#include <epoch/core.h>

namespace epoch::frontend
{
    EmulationController::EmulationController(std::shared_ptr<Emulator> emulator) : m_emulator{ std::move(emulator) }, m_frameDuration{ 1. / m_emulator->info().framesPerSecond }
    {
    }

    EmulationController::~EmulationController()
    {
        {
            std::lock_guard lock(m_mutex);
            m_shouldExit = true;
        }
        if (m_thread.joinable())
        {
            m_conditionVariable.notify_all();
            m_thread.join();
        }
    }

    void EmulationController::start()
    {
        assert(!m_thread.joinable());
        m_thread = std::thread(&EmulationController::run, this);
    }

    void EmulationController::suspend()
    {
        {
            std::lock_guard lock(m_mutex);
            m_paused = true;
        }
    }

    void EmulationController::resume()
    {
        {
            std::lock_guard lock(m_mutex);
            m_paused = false;
        }
        m_conditionVariable.notify_one();
    }

    void EmulationController::setSpeed(const double speed)
    {
        {
            std::lock_guard lock(m_mutex);
            m_speed = speed;
        }
        m_conditionVariable.notify_one();
    }

    void EmulationController::run()
    {
        while (!m_shouldExit)
        {
            int targetTimePerFrame;
            {
                std::unique_lock lock(m_mutex);
                m_conditionVariable.wait(lock, [&]() { return m_shouldExit || (!m_paused && m_speed > 0); });
                if (m_shouldExit || m_paused || m_speed <= 0)
                {
                    continue;
                }
                targetTimePerFrame = static_cast<int>((m_frameDuration * m_speed) * 1000000);
            }

            // TODO run emulation (frame by frame?) to keep it in sync with the expected speed
            const auto start = std::chrono::high_resolution_clock::now();
            m_emulator->frame();
            const auto end = start + std::chrono::microseconds(targetTimePerFrame);
            const auto now = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(1);
            if (now < end)
            {
                std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() << " " << std::chrono::duration_cast<std::chrono::milliseconds>(end.time_since_epoch()).count() << "\n";
                std::this_thread::sleep_for(end - now);
            }
            while (std::chrono::high_resolution_clock::now() < end)
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1ms);
            }

            m_timer.tick();

            // TODO store audio samples somewhere
        }
    }
}
