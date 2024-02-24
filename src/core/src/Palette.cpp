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

#include "Palette.hpp"

namespace epoch
{
    Palette::Palette(std::span<Color> colors)
    {
        m_palette.resize(colors.size());
        m_palette.assign(colors.begin(), colors.end());
    }

    Color Palette::map(const std::size_t index) const
    {
        return m_palette[index];
    }

    std::size_t Palette::size() const
    {
        return m_palette.size();
    }
}
