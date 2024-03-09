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

#ifndef SRC_EPOCH_ZXSPECTRUM_TAPEINTERFACE_HPP_
#define SRC_EPOCH_ZXSPECTRUM_TAPEINTERFACE_HPP_

#include <vector>

namespace epoch::zxspectrum
{
    class TapeInterface
    {
    public:
        explicit TapeInterface(const std::initializer_list<std::size_t> pulses) : m_pulses{pulses} {}
        explicit TapeInterface(std::vector<std::size_t> other) : m_pulses{std::move(other)} {}

        [[nodiscard]] bool clock()
        {
            if (completed()) return false;
            if (m_pulses[m_position] == 0)
            {
                m_position++;
            }
            else
            {
                m_pulses[m_position]--;
            }
            return m_position & 1;
        }
        [[nodiscard]] bool completed() const { return m_position == m_pulses.size(); }

    private:
        std::size_t m_position{};
        std::vector<std::size_t> m_pulses{};
    };
}  // namespace epoch::zxspectrum

#endif
