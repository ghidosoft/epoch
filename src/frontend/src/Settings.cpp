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

#include "Settings.hpp"

#include <fstream>

#include <yaml-cpp/yaml.h>

namespace epoch::frontend
{
    Settings::Settings()
    {
        m_path = std::filesystem::current_path() / "epoch.yaml";
    }

    void Settings::load()
    {
        reset();
        std::ifstream fin{m_path};
        if (fin.good())
        {
            try
            {
                const auto yaml = YAML::Load(fin);
                m_dirty = false;
            }
            catch (...)
            {
                reset();
            }
        }
    }

    void Settings::save()
    {
        YAML::Emitter out;

        std::ofstream fout{m_path};
        fout << out.c_str();

        m_dirty = false;
    }

    void Settings::reset()
    {
        m_dirty = true;
    }
}
