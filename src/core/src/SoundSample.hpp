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

#ifndef SRC_EPOCH_CORE_SOUNDSAMPLE_HPP_
#define SRC_EPOCH_CORE_SOUNDSAMPLE_HPP_

namespace epoch
{
    struct SoundSample final
    {
        float left;
        float right;

        constexpr SoundSample() : left{0}, right{0} {}
        constexpr SoundSample(const float mono) : left{mono}, right{mono} {}
        constexpr SoundSample(const float left, const float right) : left{left}, right{right} {}

        SoundSample& operator+=(const SoundSample& rhs)
        {
            left += rhs.left;
            right += rhs.right;
            return *this;
        }
        friend SoundSample operator+(SoundSample lhs, const SoundSample& rhs)
        {
            lhs += rhs;
            return lhs;
        }

        SoundSample& operator-=(const SoundSample& rhs)
        {
            left -= rhs.left;
            right -= rhs.right;
            return *this;
        }
        friend SoundSample operator-(SoundSample lhs, const SoundSample& rhs)
        {
            lhs -= rhs;
            return lhs;
        }

        SoundSample& operator*=(const SoundSample& rhs)
        {
            left *= rhs.left;
            right *= rhs.right;
            return *this;
        }
        friend SoundSample operator*(SoundSample lhs, const SoundSample& rhs)
        {
            lhs *= rhs;
            return lhs;
        }

        SoundSample& operator+=(const float rhs)
        {
            left += rhs;
            right += rhs;
            return *this;
        }
        friend SoundSample operator+(SoundSample lhs, const float rhs)
        {
            lhs += rhs;
            return lhs;
        }

        SoundSample& operator-=(const float rhs)
        {
            left -= rhs;
            right -= rhs;
            return *this;
        }
        friend SoundSample operator-(SoundSample lhs, const float rhs)
        {
            lhs -= rhs;
            return lhs;
        }

        SoundSample& operator*=(const float rhs)
        {
            left *= rhs;
            right *= rhs;
            return *this;
        }
        friend SoundSample operator*(SoundSample lhs, const float rhs)
        {
            lhs *= rhs;
            return lhs;
        }
    };
}  // namespace epoch

#endif
