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

#ifndef SRC_EPOCH_ZXSPECTRUM_PULSESTAPE_HPP_
#define SRC_EPOCH_ZXSPECTRUM_PULSESTAPE_HPP_

#include <epoch/core.hpp>

#include <vector>

namespace epoch::zxspectrum
{
    class PulsesTape final : public Tape
    {
    public:
        explicit PulsesTape(const std::initializer_list<std::size_t> pulses) : m_pulses{pulses} {}
        explicit PulsesTape(std::vector<std::size_t> other) : m_pulses{std::move(other)} {}

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

        void play() override { m_playing = true; }
        void stop() override { m_playing = false; }
        [[nodiscard]] bool playing() const override { return m_playing; }

    private:
        std::size_t m_position{};
        std::vector<std::size_t> m_pulses{};

        bool m_playing{true};
    };
}  // namespace epoch::zxspectrum

#endif
