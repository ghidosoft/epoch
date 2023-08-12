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

#ifndef SRC_EPOCH_ZXSPECTRUM_TOOLS_UTILS_HPP_
#define SRC_EPOCH_ZXSPECTRUM_TOOLS_UTILS_HPP_

#include <cassert>
#include <span>

#include "../src/Z80Cpu.hpp"

class RamZ80Interface : public epoch::zxspectrum::Z80Interface
{
public:
    RamZ80Interface() = default;
    explicit RamZ80Interface(const std::span<uint8_t> ram)
    {
        assert(ram.size() <= m_ram.size());
        std::memcpy(m_ram.data(), ram.data(), ram.size());
    }

    uint8_t read(const uint16_t address) override
    {
        return m_ram[address];
    }
    void write(const uint16_t address, const uint8_t value) override
    {
        m_ram[address] = value;
    }

    uint8_t ioRead(uint16_t port) override { return 0; }
    void ioWrite(uint16_t port, uint8_t value) override {}

    std::span<uint8_t> ram() { return m_ram; }

private:
    std::array<uint8_t, 0x10000> m_ram;
};

#endif
