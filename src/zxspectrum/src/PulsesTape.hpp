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

#include <limits>
#include <vector>

namespace epoch::zxspectrum
{
    class PulsesTape final : public Tape
    {
    public:
        using pulse_t = int64_t;

        static constexpr pulse_t StopTape = -1;

    public:
        explicit PulsesTape(const std::initializer_list<pulse_t> pulses) : m_pulses{pulses} { next(); }
        explicit PulsesTape(std::vector<pulse_t> other) : m_pulses{std::move(other)} { next(); }

        [[nodiscard]] bool clock()
        {
            if (completed()) return false;
            if (m_current > 0)
            {
                m_current--;
            }
            else if (m_current == 0)
            {
                next();
            }
            else if (m_current == StopTape)
            {
                next();
                stop();
                return (m_position - 1) & 1;
            }
            return m_position & 1;
        }
        [[nodiscard]] bool completed() const { return m_position == m_pulses.size(); }

        void play() override { m_playing = true; }
        void stop() override { m_playing = false; }
        [[nodiscard]] bool playing() const override { return m_playing; }

    private:
        std::size_t m_position{std::numeric_limits<size_t>::max()};
        std::vector<pulse_t> m_pulses;
        pulse_t m_current{};

        bool m_playing{true};

        void next()
        {
            m_position++;
            if (completed() == false)
            {
                m_current = m_pulses[m_position];
            }
        }
    };
}  // namespace epoch::zxspectrum

#endif
