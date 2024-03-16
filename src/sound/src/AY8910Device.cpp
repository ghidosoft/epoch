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

#include "AY8910Device.hpp"

#include <cassert>

namespace epoch::sound
{
    AY8910Device::AY8910Device(const std::size_t frequency) : m_frequency{frequency} {}

    void AY8910Device::reset()
    {
        m_address = 0;
        m_data = 0;
    }

    void AY8910Device::clock() {}

    void AY8910Device::address(const uint8_t value)
    {
        assert(value < m_registers.size());
        m_address = value;
    }

    void AY8910Device::data(const uint8_t data) { m_registers[m_address] = data; }

    uint8_t AY8910Device::data() const { return m_registers[m_address]; }

    float AY8910Device::output() const { return 0.f; }
}  // namespace epoch::sound
