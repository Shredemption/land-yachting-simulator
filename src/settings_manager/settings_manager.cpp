#include "settings_manager/settings_manager.hpp"

#include "pch.h"

void SettingsManager::load()
{
    std::ifstream file(settingsFile);
    if (!file.is_open())
    {
        save();
    }

    try
    {
        jsoncons::json j = jsoncons::json::parse(file);
        settings = j.as<SettingsStruct>();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading settings: " << e.what() << "\n";
    }
}

void SettingsManager::save()
{
    std::ofstream file(settingsFile);
    if (!file.is_open())
    {
        std::cerr << "Cannot open file for writing: " << settingsFile << "\n";
        return;
    }

    try
    {
        jsoncons::json j = settings;
        file << jsoncons::pretty_print(j) << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error serializing settings: " << e.what() << "\n";
    }
}