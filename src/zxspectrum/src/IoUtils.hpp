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

#ifndef SRC_EPOCH_ZXSPECTRUM_IOUTILS_HPP_
#define SRC_EPOCH_ZXSPECTRUM_IOUTILS_HPP_

#include <istream>

namespace epoch::zxspectrum
{
    class StreamReader final
    {
    public:
        explicit StreamReader(std::istream& stream);
        ~StreamReader();

    public:
        StreamReader(const StreamReader& other) = delete;
        StreamReader(StreamReader&& other) noexcept = delete;
        StreamReader& operator=(const StreamReader& other) = delete;
        StreamReader& operator=(StreamReader&& other) noexcept = delete;

    public:
        uint8_t readUInt8() const;
        uint16_t readUInt16LE() const;

    private:
        std::istream& m_stream;
    };
}  // namespace epoch::zxspectrum

#endif
