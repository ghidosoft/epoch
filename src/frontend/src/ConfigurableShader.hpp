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

#ifndef SRC_EPOCH_FRONTEND_CONFIGURABLESHADER_HPP_
#define SRC_EPOCH_FRONTEND_CONFIGURABLESHADER_HPP_

#include <string>
#include <vector>

namespace epoch::frontend
{
    struct ConfigurableShaderParameter
    {
        ConfigurableShaderParameter(std::string variableName, std::string description, float defaultValue, float min, float max, float step, float value);

        std::string variableName;
        std::string description;
        float defaultValue;
        float min;
        float max;
        float step;
        float value;
    };

    class ConfigurableShader final
    {
    public:
        ConfigurableShader(std::string key, std::string name, std::string_view source);

        [[nodiscard]] const std::string& key() const;
        [[nodiscard]] const std::string& name() const;
        [[nodiscard]] const std::string& source() const;
        [[nodiscard]] std::vector<ConfigurableShaderParameter>& parameters();

    private:
        std::string m_key;
        std::string m_name;
        std::string m_source;
        std::vector<ConfigurableShaderParameter> m_parameters;
    };
}

#endif
