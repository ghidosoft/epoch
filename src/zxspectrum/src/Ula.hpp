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

#ifndef SRC_EPOCH_ZXSPECTRUM_ULA_HPP_
#define SRC_EPOCH_ZXSPECTRUM_ULA_HPP_

#include "Constants.hpp"
#include "Z80Interface.hpp"

#include <epoch/core.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <span>

namespace epoch::sound
{
    class AY8910Device;
}

namespace epoch::zxspectrum
{
    class FloppyUpd765;

    enum class UlaType
    {
        zx48k,
        zx128k,
        zx128kplus3,
    };

    class Ula final : public Z80Interface
    {
    public:
        Ula(UlaType type, std::span<const uint8_t> rom);
        ~Ula() override;

    public:
        Ula(const Ula& other) = delete;
        Ula(Ula&& other) noexcept = delete;
        Ula& operator=(const Ula& other) = delete;
        Ula& operator=(Ula&& other) noexcept = delete;

    public:
        void clock();
        void reset();

        [[nodiscard]] std::array<MemoryBank, 8>& ram() { return m_ram; }
        [[nodiscard]] const std::array<MemoryBank, 8>& ram() const { return m_ram; }

        uint8_t read(uint16_t address) override;
        void write(uint16_t address, uint8_t value) override;
        uint8_t ioRead(uint16_t port) override;
        void ioWrite(uint16_t port, uint8_t value) override;

        [[nodiscard]] bool isCpuStalled() const { return m_cpuStalled > 0; }

        [[nodiscard]] std::span<const uint8_t> screenBuffer() const { return m_screenBuffer; }

        [[nodiscard]] bool interruptRequested() const
        {
            return m_y == -VerticalRetrace && m_x >= BorderLeft && m_x < BorderLeft + InterruptActiveTStates * 2;
        }
        [[nodiscard]] bool frameReady() const { return m_y == -VerticalRetrace && m_x == -HorizontalRetrace; }

        [[nodiscard]] SoundSample audioOutput() const;
        [[nodiscard]] uint8_t border() const { return m_border; }

        void setKeyState(int row, int col, bool state);
        void setKempstonState(int button, bool state);
        void setAudioIn(const bool value) { m_audioIn = value; }

    private:
        UlaType m_type;

        std::array<MemoryBank, 4> m_rom{};
        std::array<MemoryBank, 8> m_ram{};

        std::unique_ptr<sound::AY8910Device> m_ay8910{};
        std::unique_ptr<FloppyUpd765> m_floppy{};

        uint8_t m_ramSelect{0};
        uint8_t m_vramSelect{5};
        std::span<uint8_t> m_vram{m_ram[m_vramSelect]};
        uint8_t m_romSelect{0};
        uint8_t m_pagingState{};
        uint8_t m_pagingPlus3{};

        uint8_t m_floatingBusValue{};
        uint8_t m_border{};
        std::array<uint8_t, 8> m_keyboardState{};  // Rows 0=Caps, A, Q, 1, 6, Y, H, 7=B
        bool m_ear{}, m_mic{}, m_audioIn{};
        uint8_t m_kempstonState{};
        int m_cpuStalled{};

        uint64_t m_clockCounter{};
        uint64_t m_frameCounter{};
        std::array<uint8_t, static_cast<std::size_t>(Width* Height)> m_screenBuffer{};
        int m_x{-HorizontalRetrace}, m_y{-VerticalRetrace};
    };
}  // namespace epoch::zxspectrum

#endif
