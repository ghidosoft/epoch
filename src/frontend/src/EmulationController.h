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

#include <atomic>
#include <condition_variable>
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
        void pause();
        void resume();

    private:
        void run();

    private:
        std::shared_ptr<Emulator> m_emulator;
        std::atomic<bool> m_shouldExit{};

        bool m_paused{};

        std::thread m_thread{};
        std::mutex m_mutex{};
        std::condition_variable m_conditionVariable;

        CircularBuffer<float, 16384> m_audioBuffer{};
    };
}

#endif
