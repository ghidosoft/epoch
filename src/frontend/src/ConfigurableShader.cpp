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

#include "ConfigurableShader.hpp"

#include <iomanip>
#include <sstream>

namespace epoch::frontend
{
    ConfigurableShaderParameter::ConfigurableShaderParameter(
        std::string variableName, std::string description,
        const float defaultValue, const float min, const float max, const float step, const float value
    ) : variableName(std::move(variableName)), description(std::move(description)), defaultValue(defaultValue), min(min), max(max), step(step), value(value)
    {
    }

    ConfigurableShader::ConfigurableShader(std::string key, std::string name, const std::string_view source) : m_key{ std::move(key) }, m_name{ std::move(name) }, m_source { source }
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

    const std::string& ConfigurableShader::key() const
    {
        return m_key;
    }

    const std::string& ConfigurableShader::name() const
    {
        return m_name;
    }

    const std::string &ConfigurableShader::source() const
    {
        return m_source;
    }

    std::vector<ConfigurableShaderParameter>& ConfigurableShader::parameters()
    {
        return m_parameters;
    }
}
