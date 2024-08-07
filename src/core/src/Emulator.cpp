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

#include "Emulator.hpp"

#include <cassert>

namespace epoch
{
    Emulator::Emulator(EmulatorInfo info) : m_info{std::move(info)}
    {
        assert(m_info.frameClocks > 0);
        assert(m_info.width > 0);
        assert(m_info.height > 0);
        assert(m_info.framesPerSecond > 0.0);
        m_clockDuration = 1.0 / (static_cast<double>(m_info.frameClocks) * m_info.framesPerSecond);
    }

    Emulator::~Emulator() = default;

    void Emulator::clock()
    {
        doClock();
        m_elapsed += m_clockDuration;
    }

    void Emulator::frame()
    {
        for (std::size_t i = 0; i < m_info.frameClocks; i++)
        {
            clock();
        }
    }

    SoundSample Emulator::generateNextAudioSample()
    {
        constexpr auto sampleDuration = 1.0 / 48000.0;  // TODO: hardcoded sample rate
        while (m_elapsed < sampleDuration)
        {
            clock();
        }
        m_elapsed -= sampleDuration;
        return audioOut();
    }

    const EmulatorInfo &Emulator::info() const { return m_info; }
}  // namespace epoch
