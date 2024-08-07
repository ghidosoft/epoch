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

#ifndef SRC_EPOCH_ZXSPECTRUM_ZXSPECTRUMEMULATOR_HPP_
#define SRC_EPOCH_ZXSPECTRUM_ZXSPECTRUMEMULATOR_HPP_

#include "Constants.hpp"

#include <epoch/core.hpp>

#include <array>
#include <memory>

namespace epoch::zxspectrum
{
    class PulsesTape;
    class Ula;
    class Z80Cpu;

    class ZXSpectrumEmulator final : public Emulator
    {
    public:
        explicit ZXSpectrumEmulator(std::unique_ptr<Ula> ula);
        ~ZXSpectrumEmulator() override;

    public:
        static std::unique_ptr<ZXSpectrumEmulator> create48K();
        static std::unique_ptr<ZXSpectrumEmulator> create128K();
        static std::unique_ptr<ZXSpectrumEmulator> create128KPlus2();
        static std::unique_ptr<ZXSpectrumEmulator> create128KPlus3();

    public:
        ZXSpectrumEmulator(const ZXSpectrumEmulator& other) = delete;
        ZXSpectrumEmulator(ZXSpectrumEmulator&& other) noexcept = delete;
        ZXSpectrumEmulator& operator=(const ZXSpectrumEmulator& other) = delete;
        ZXSpectrumEmulator& operator=(ZXSpectrumEmulator&& other) noexcept = delete;

    public:
        void reset() override;

        void load(const std::string& path) override;
        void save(const std::string& path) override;

        [[nodiscard]] std::span<const uint32_t> screenBuffer() override { return m_screenBuffer; }

        [[nodiscard]] SoundSample audioOut() const override;

        void keyEvent(Key key, KeyAction action) override;

        [[nodiscard]] Z80Cpu* cpu() const { return m_cpu.get(); }
        [[nodiscard]] Ula* ula() const { return m_ula.get(); }
        [[nodiscard]] std::array<MemoryBank, 8>& ram();
        [[nodiscard]] const std::array<MemoryBank, 8>& ram() const;

        Tape* tape() override;

    protected:
        void doClock() override;

    private:
        const std::unique_ptr<Ula> m_ula;
        const std::unique_ptr<Z80Cpu> m_cpu;
        uint64_t m_clockCounter{};

        std::array<uint32_t, 16> m_palette;

        std::array<uint32_t, static_cast<std::size_t>(Width* Height)> m_screenBuffer{};

        std::unique_ptr<PulsesTape> m_tape{};

        void updateScreenBuffer();
    };
}  // namespace epoch::zxspectrum

#endif
