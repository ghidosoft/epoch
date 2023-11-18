/* This file is part of Epoch, Copyright (C) 2023 Andrea Ghidini.
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

#ifndef SRC_FRONTEND_PLATFORM_HPP_
#define SRC_FRONTEND_PLATFORM_HPP_

#include <span>
#include <string>

#include <epoch/core.hpp>

namespace epoch::frontend
{
    class Platform final
    {
    public:
        Platform();
        ~Platform();

    public:
        Platform(const Platform& other) = delete;
        Platform(Platform&& other) noexcept = delete;
        Platform& operator=(const Platform& other) = delete;
        Platform& operator=(Platform&& other) noexcept = delete;

    public:
        [[nodiscard]] std::string openDialog(std::span<const EmulatorFileFormat> = {}) const;
        [[nodiscard]] std::string saveDialog(std::span<const EmulatorFileFormat> = {}) const;
    };
}

#endif
