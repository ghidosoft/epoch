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

#include <cstdint>

namespace epoch::zxspectrum
{
    struct FloppyUpd765Flags
    {
        enum Values : uint8_t
        {
            fdd0 = 1 << 0,
            fdd1 = 1 << 1,
            fdd2 = 1 << 2,
            fdd3 = 1 << 3,
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

        [[nodiscard]] bool fdd0() const { return value & FloppyUpd765Flags::fdd0; }
        [[nodiscard]] bool fdd1() const { return value & FloppyUpd765Flags::fdd1; }
        [[nodiscard]] bool fdd2() const { return value & FloppyUpd765Flags::fdd2; }
        [[nodiscard]] bool fdd3() const { return value & FloppyUpd765Flags::fdd3; }
        [[nodiscard]] bool cb() const { return value & FloppyUpd765Flags::cb; }
        [[nodiscard]] bool exm() const { return value & FloppyUpd765Flags::exm; }
        [[nodiscard]] bool dio() const { return value & FloppyUpd765Flags::dio; }
        [[nodiscard]] bool rqm() const { return value & FloppyUpd765Flags::rqm; }

        void fdd0(const bool f) { value = (value & ~FloppyUpd765Flags::fdd0) | static_cast<uint8_t>(f << 0); }
        void fdd1(const bool f) { value = (value & ~FloppyUpd765Flags::fdd1) | static_cast<uint8_t>(f << 1); }
        void fdd2(const bool f) { value = (value & ~FloppyUpd765Flags::fdd2) | static_cast<uint8_t>(f << 2); }
        void fdd3(const bool f) { value = (value & ~FloppyUpd765Flags::fdd3) | static_cast<uint8_t>(f << 3); }
        void cb(const bool f) { value = (value & ~FloppyUpd765Flags::cb) | static_cast<uint8_t>(f << 4); }
        void exm(const bool f) { value = (value & ~FloppyUpd765Flags::exm) | static_cast<uint8_t>(f << 5); }
        void dio(const bool f) { value = (value & ~FloppyUpd765Flags::dio) | static_cast<uint8_t>(f << 6); }
        void rqm(const bool f) { value = (value & ~FloppyUpd765Flags::rqm) | static_cast<uint8_t>(f << 7); }

        operator uint8_t() const { return value; }
    };

    class FloppyUpd765 final
    {
    public:
        void reset();

        [[nodiscard]] uint8_t status() const { return m_statusRegister; }

        [[nodiscard]] uint8_t read();
        void write(uint8_t data);

    private:
        FloppyUpd765StatusRegister m_statusRegister{};
    };
}  // namespace epoch::zxspectrum

#endif
