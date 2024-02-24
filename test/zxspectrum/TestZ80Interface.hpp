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

#ifndef TEST_EPOCH_ZXSPECTRUM_TESTZ80INTERFACE_H_
#define TEST_EPOCH_ZXSPECTRUM_TESTZ80INTERFACE_H_

#include <array>
#include <cassert>
#include <cstring>
#include <span>

#include "../../src/zxspectrum/src/Z80Cpu.hpp"

namespace epoch::zxspectrum
{
    class TestZ80Interface : public Z80Interface
    {
    public:
        TestZ80Interface() = default;
        TestZ80Interface(const std::span<const uint8_t> initRam)
        {
            assert(initRam.size_bytes() <= m_ram.size());
            std::memcpy(m_ram.data(), initRam.data(), initRam.size_bytes());
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

        [[nodiscard]] std::span<uint8_t> ram() { return m_ram; }
        [[nodiscard]] uint8_t ram(const uint16_t address) const { return m_ram[address]; }

    private:
        std::array<uint8_t, 0x10000> m_ram{};
    };
}

#endif
