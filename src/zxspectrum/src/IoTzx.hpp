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

#ifndef SRC_EPOCH_ZXSPECTRUM_IO_TZX_HPP_
#define SRC_EPOCH_ZXSPECTRUM_IO_TZX_HPP_

#include <istream>
#include <vector>

namespace epoch::zxspectrum
{
    class TzxReader
    {
    public:
        TzxReader(std::istream& stream, std::vector<std::size_t>& pulses);
        ~TzxReader();

    public:
        TzxReader(const TzxReader& other) = delete;
        TzxReader(TzxReader&& other) noexcept = delete;
        TzxReader& operator=(const TzxReader& other) = delete;
        TzxReader& operator=(TzxReader&& other) noexcept = delete;

    public:
        void read();

    private:
        void loadBlock(uint8_t blockId);
        void loadBlock10StandardSpeed();
        void loadBlock11TurboSpeed();
        void loadBlock12PureTone();
        void loadBlock13PulseSequence();
        void loadBlock14PureDataBlock();

    private:
        std::istream& m_stream;
        std::vector<std::size_t>& m_pulses;

        uint16_t m_loopCount{};
        std::istream::pos_type m_loopPos{};
    };

    void loadTzx(std::istream& is, std::vector<std::size_t>& pulses);
}  // namespace epoch::zxspectrum

#endif
