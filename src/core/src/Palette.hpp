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

#ifndef SRC_EPOCH_CORE_PALETTE_HPP_
#define SRC_EPOCH_CORE_PALETTE_HPP_

#include "Color.hpp"

#include <span>
#include <vector>

namespace epoch
{
    class Palette final
    {
    public:
        explicit Palette(std::span<Color> colors);

        [[nodiscard]] Color map(std::size_t index) const;
        [[nodiscard]] std::size_t size() const;

    private:
        std::vector<Color> m_palette{};
    };
}  // namespace epoch

#endif
