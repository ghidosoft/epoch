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

#include "Platform.hpp"

#include <stdexcept>

#include <nfd.hpp>

namespace epoch::frontend
{
    Platform::Platform()
    {
        if (NFD::Init() != NFD_OKAY)
        {
            throw std::runtime_error("Cannot initialize NFD library");
        }
    }

    Platform::~Platform()
    {
        NFD::Quit();
    }

    std::string Platform::openDialog() const
    {
        NFD::UniquePath path{};
        const auto result = NFD::OpenDialog(path);
        if (result == NFD_OKAY)
        {
            return path.get();
        }
        return {};
    }
}
