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

#ifndef SRC_EPOCH_FLOPPYUPD765_IO_HPP_
#define SRC_EPOCH_FLOPPYUPD765_IO_HPP_

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace epoch::zxspectrum
{
    struct FloppyUpd765Flags
    {
        enum Values : uint8_t
        {
            d0b = 1 << 0,
            d1b = 1 << 1,
            d2b = 1 << 2,
            d3b = 1 << 3,
            cb = 1 << 4,
            exm = 1 << 5,
            dio = 1 << 6,
            rqm = 1 << 7,
        };
    };

    struct FloppyUpd765StatusRegister
    {
        uint8_t value{FloppyUpd765Flags::rqm};

        FloppyUpd765StatusRegister() = default;
        FloppyUpd765StatusRegister(const uint8_t v) : value{v} {}

        [[nodiscard]] bool d0b() const { return value & FloppyUpd765Flags::d0b; }
        [[nodiscard]] bool d1b() const { return value & FloppyUpd765Flags::d1b; }
        [[nodiscard]] bool d2b() const { return value & FloppyUpd765Flags::d2b; }
        [[nodiscard]] bool d3b() const { return value & FloppyUpd765Flags::d3b; }
        [[nodiscard]] bool cb() const { return value & FloppyUpd765Flags::cb; }
        [[nodiscard]] bool exm() const { return value & FloppyUpd765Flags::exm; }
        [[nodiscard]] bool dio() const { return value & FloppyUpd765Flags::dio; }
        [[nodiscard]] bool rqm() const { return value & FloppyUpd765Flags::rqm; }

        void d0b(const bool f) { value = (value & ~FloppyUpd765Flags::d0b) | static_cast<uint8_t>(f << 0); }
        void d1b(const bool f) { value = (value & ~FloppyUpd765Flags::d1b) | static_cast<uint8_t>(f << 1); }
        void d2b(const bool f) { value = (value & ~FloppyUpd765Flags::d2b) | static_cast<uint8_t>(f << 2); }
        void d3b(const bool f) { value = (value & ~FloppyUpd765Flags::d3b) | static_cast<uint8_t>(f << 3); }
        void cb(const bool f) { value = (value & ~FloppyUpd765Flags::cb) | static_cast<uint8_t>(f << 4); }
        void exm(const bool f) { value = (value & ~FloppyUpd765Flags::exm) | static_cast<uint8_t>(f << 5); }
        void dio(const bool f) { value = (value & ~FloppyUpd765Flags::dio) | static_cast<uint8_t>(f << 6); }
        void rqm(const bool f) { value = (value & ~FloppyUpd765Flags::rqm) | static_cast<uint8_t>(f << 7); }

        operator uint8_t() const { return value; }
    };

    struct FloppyImage final
    {
        std::vector<uint8_t> data;
    };

    class FloppyDrive final
    {
    public:
        void seek(uint8_t cylinder);
        void load(std::shared_ptr<FloppyImage> image);
        void eject();

        [[nodiscard]] uint8_t cylinder() const { return m_cylinder; }
        [[nodiscard]] uint8_t track() const { return m_track; }
        [[nodiscard]] bool hasImage() const { return m_image != nullptr; }

    private:
        std::shared_ptr<FloppyImage> m_image{};
        uint8_t m_cylinder{};
        uint8_t m_track{};
    };

    class FloppyUpd765 final
    {
    public:
        static constexpr auto SectorSize = 256;
        static constexpr auto SectorsPerTrack = 16;

    public:
        FloppyUpd765();

    public:
        void reset();

        [[nodiscard]] uint8_t status() const { return m_statusRegister; }

        [[nodiscard]] uint8_t read();
        void write(uint8_t data);

        void load(std::shared_ptr<FloppyImage> image);
        void eject();

    private:
        FloppyUpd765StatusRegister m_statusRegister{};
        std::vector<uint8_t> m_input{};
        std::vector<uint8_t> m_output{};
        std::array<FloppyDrive, 4> m_drives{};

        uint8_t m_lastDrive{};

        void specify();
        void senseDriveStatus();
        void recalibrate();
        void senseInterruptStatus();
        void readId();
        void seek();
        void invalid();

        void finishCommand();
    };
}  // namespace epoch::zxspectrum

#endif
