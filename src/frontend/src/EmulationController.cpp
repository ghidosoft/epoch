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
    EmulationController::EmulationController(std::shared_ptr<Emulator> emulator) : m_emulator{ std::move(emulator) }
    {
    }

    EmulationController::~EmulationController()
    {
        m_shouldExit = true;
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

    void EmulationController::start()
    {
        assert(!m_thread.joinable());
        m_thread = std::thread(&EmulationController::run, this);
    }

    void EmulationController::run()
    {
        using namespace std::chrono_literals;

        while (!m_shouldExit)
        {
            std::unique_lock lock(m_mutex);
            m_conditionVariable.wait_for(lock, 20ms);

            // TODO
        }
    }
}
