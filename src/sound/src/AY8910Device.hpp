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

#ifndef SRC_SOUND_AY8910DEVICE_HPP_
#define SRC_SOUND_AY8910DEVICE_HPP_

#include "SoundDevice.hpp"

#include <array>
#include <cstdint>

namespace epoch::sound
{
    class AY8910Device : public SoundDevice
    {
    public:
        AY8910Device();

    public:
        void reset() override;
        virtual void clock();

        void address(uint8_t value);
        void data(uint8_t data);
        [[nodiscard]] uint8_t data() const;

    private:
        std::array<uint8_t, 16> m_registers{};
        uint8_t m_address{};
        uint8_t m_data{};
    };
}  // namespace epoch::sound

#endif
