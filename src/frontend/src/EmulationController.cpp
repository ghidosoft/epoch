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
            double targetTimePerFrame;
            {
                std::unique_lock lock(m_mutex);
                m_conditionVariable.wait(lock, [&]() { return m_shouldExit || (!m_paused && m_speed > 0); });
                if (m_shouldExit || m_paused || m_speed <= 0)
                {
                    continue;
                }
                targetTimePerFrame = m_frameDuration * m_speed;
            }

            // TODO run emulation (frame by frame?) to keep it in sync with the expected speed
            // const auto start = std::chrono::high_resolution_clock::now();
            m_emulator->frame();
            // const auto end = std::chrono::high_resolution_clock::now();

            m_timer.tick();

            if (m_timer.tickDuration() < targetTimePerFrame)
            {
                const auto sleepFor = std::chrono::duration<double>(targetTimePerFrame - m_timer.tickDuration());
                std::this_thread::sleep_for(sleepFor);
            }

            /* const auto elapsed = std::chrono::duration<double, std::ratio<1>>(end - start).count();
            if (elapsed < targetTimePerFrame)
            {
                const auto sleepFor = std::chrono::duration<double>(targetTimePerFrame - elapsed);
                std::this_thread::sleep_for(sleepFor);
            } */

            // TODO store audio samples somewhere
        }
    }
}
