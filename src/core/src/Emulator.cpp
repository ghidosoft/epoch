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

#include "Emulator.h"

#include <cassert>

namespace epoch
{
    Emulator::Emulator(EmulatorInfo info) : m_info{ std::move(info) }
    {
        assert(m_info.frameClocks > 0);
        assert(m_info.width > 0);
        assert(m_info.height > 0);
        assert(m_info.framesPerSecond > 0.0);
    }

    Emulator::~Emulator() = default;

    void Emulator::frame()
    {
        for (std::size_t i = 0; i < m_info.frameClocks; i++)
        {
            clock();
        }
    }

    const EmulatorInfo &Emulator::info() const
    {
        return m_info;
    }
}
