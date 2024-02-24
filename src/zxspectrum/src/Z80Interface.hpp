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

#ifndef SRC_EPOCH_ZXSPECTRUM_Z80INTERFACE_HPP_
#define SRC_EPOCH_ZXSPECTRUM_Z80INTERFACE_HPP_

#include <cstdint>

namespace epoch::zxspectrum
{
    class Z80Interface
    {
    public:
        virtual ~Z80Interface() = default;
        virtual uint8_t read(uint16_t address) = 0;
        virtual void write(uint16_t address, uint8_t value) = 0;
        virtual uint8_t ioRead(uint16_t port) = 0;
        virtual void ioWrite(uint16_t port, uint8_t value) = 0;
    };
}

#endif
