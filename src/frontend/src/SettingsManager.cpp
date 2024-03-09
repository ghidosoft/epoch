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

#include "SettingsManager.hpp"

#include <fstream>

#include <yaml-cpp/yaml.h>

template<>
struct YAML::convert<epoch::frontend::SettingsEmulator>
{
    static Node encode(const epoch::frontend::SettingsEmulator& rhs)
    {
        Node node;
        node["key"] = rhs.key;
        return node;
    }

    static bool decode(const Node& node, epoch::frontend::SettingsEmulator& rhs)
    {
        if (!node.IsMap())
        {
            return false;
        }
        rhs.key = node["key"].as<std::string>("");
        return true;
    }
};

template<>
struct YAML::convert<epoch::frontend::SettingsUI>
{
    static Node encode(const epoch::frontend::SettingsUI& rhs)
    {
        Node node;
        node["imgui"] = rhs.imgui;
        node["lastLoadPath"] = rhs.lastLoadPath;
        node["lastSavePath"] = rhs.lastSavePath;
        node["shader"] = rhs.shader;
        return node;
    }

    static bool decode(const Node& node, epoch::frontend::SettingsUI& rhs)
    {
        if (!node.IsMap())
        {
            return false;
        }
        rhs.imgui = node["imgui"].as<std::string>("");
        rhs.lastLoadPath = node["lastLoadPath"].as<std::string>(".");
        rhs.lastSavePath = node["lastSavePath"].as<std::string>(".");
        rhs.shader = node["shader"].as<std::string>("");
        return true;
    }
};

namespace epoch::frontend
{
    SettingsManager::SettingsManager() : m_path{ std::filesystem::current_path() / "epoch.yaml" }
    {
    }

    SettingsManager::~SettingsManager() = default;

    void SettingsManager::load()
    {
        reset();
        std::ifstream fin{ m_path };
        if (fin.good())
        {
            try
            {
                const auto node = YAML::Load(fin);
                m_currentSettings.emulator = node["emulator"].as<SettingsEmulator>();
                m_currentSettings.ui = node["ui"].as<SettingsUI>();
                m_storedSettings = m_currentSettings;
            }
            catch (...)
            {
                reset();
            }
        }
    }

    void SettingsManager::save()
    {
        std::ofstream fout{ m_path };
        if (fout.good())
        {
            YAML::Node node;
            node["emulator"] = m_currentSettings.emulator;
            node["ui"] = m_currentSettings.ui;

            fout << node;

            m_storedSettings = m_currentSettings;
        }
    }

    void SettingsManager::reset()
    {
        m_currentSettings = {};
    }

    bool SettingsManager::dirty() const
    {
        return m_currentSettings != m_storedSettings;
    }
}
