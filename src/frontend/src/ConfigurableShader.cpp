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

#include "ConfigurableShader.hpp"

#include <iomanip>
#include <sstream>

namespace epoch::frontend
{
    ConfigurableShader::ConfigurableShader(std::string name, const std::string_view source) : m_source{ source }, m_name{ std::move(name) }
    {
        std::istringstream f{ m_source };
        std::string line;
        while (std::getline(f, line))
        {
            if (line.starts_with("#pragma parameter "))
            {
                std::istringstream ls{ line.substr(sizeof("#pragma parameter ") - 1) };
                std::string variableName, description;
                float defaultValue, min, max, step;
                ls >> variableName;
                ls >> std::quoted(description);
                ls >> defaultValue;
                ls >> min;
                ls >> max;
                ls >> step;
                m_parameters.emplace_back(variableName, description, defaultValue, min, max, step, defaultValue);
            }
        }
    }

    const std::string &ConfigurableShader::source() const
    {
        return m_source;
    }

    const std::string &ConfigurableShader::name() const
    {
        return m_name;
    }

    std::vector<ConfigurableShaderParameter>& ConfigurableShader::parameters()
    {
        return m_parameters;
    }
}
