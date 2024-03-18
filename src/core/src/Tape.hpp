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

#ifndef SRC_EPOCH_CORE_TAPE_HPP_
#define SRC_EPOCH_CORE_TAPE_HPP_

namespace epoch
{
    class Tape
    {
    public:
        Tape() = default;
        virtual ~Tape() = default;

    public:
        Tape(const Tape& other) = default;
        Tape(Tape&& other) noexcept = default;
        Tape& operator=(const Tape& other) = default;
        Tape& operator=(Tape&& other) noexcept = default;

    public:
        virtual void play() = 0;
        virtual void stop() = 0;
        [[nodiscard]] virtual bool playing() const = 0;
    };
}  // namespace epoch

#endif
