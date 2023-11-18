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

    std::string Platform::openDialog(const std::span<const EmulatorFileFormat> formats) const
    {
        std::string all;
        std::vector<nfdu8filteritem_t> filterItems;
        for (const auto& format : formats)
        {
            if (format.load)
            {
                filterItems.push_back({
                    format.description.c_str(),
                    format.extensions.c_str(),
                    });
            }
        }
        if (filterItems.size() > 1)
        {
            for (const auto& [name, spec] : filterItems)
            {
                if (!all.empty()) all += ",";
                all += spec;
            }
            filterItems.insert(filterItems.begin(), {
                "Supported formats",
                all.c_str(),
            });
        }

        NFD::UniquePath path{};
        const auto result = NFD::OpenDialog(path, filterItems.data(), filterItems.size());
        if (result == NFD_OKAY)
        {
            return path.get();
        }
        return {};
    }

    std::string Platform::saveDialog(const std::span<const EmulatorFileFormat> formats) const
    {
        std::string all;
        std::vector<nfdu8filteritem_t> filterItems;
        for (const auto& format : formats)
        {
            if (format.save)
            {
                filterItems.push_back({
                    format.description.c_str(),
                    format.extensions.c_str(),
                });
            }
        }
        if (filterItems.size() > 1)
        {
            for (const auto& [name, spec] : filterItems)
            {
                if (!all.empty()) all += ",";
                all += spec;
            }
            filterItems.insert(filterItems.begin(), {
                "Supported formats",
                all.c_str(),
                });
        }

        NFD::UniquePath path{};
        const auto result = NFD::SaveDialog(path, filterItems.data(), filterItems.size());
        if (result == NFD_OKAY)
        {
            return path.get();
        }
        return {};
    }
}
