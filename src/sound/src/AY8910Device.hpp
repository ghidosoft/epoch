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

namespace epoch::sound
{
    class AY8910Device final : public SoundDevice
    {
    public:
        AY8910Device();

    public:
        void reset() override;
    };
}  // namespace epoch::sound

#endif
