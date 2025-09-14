#include "settings_manager/settings_manager.hpp"

#include "pch.h"

template<typename T>
void validateLimit(T& value, const Limit<T>& limit)
{
    value = std::clamp(value, limit.min, limit.max);
}

void validate(SettingsStruct& s, const SettingsMetaStruct& m)
{
    validateLimit(s.video.fov, m.video.fov);
    validateLimit(s.video.lodDistance, m.video.lodDistance);
    validateLimit(s.video.waterFrameRate, m.video.waterFrameRate);

    validateLimit(s.input.mouseSensitivity, m.input.mouseSensitivity);
    validateLimit(s.input.controllerCamSensitivity, m.input.controllerCamSensitivity);

    validateLimit(s.physics.tickRate, m.physics.tickRate);
}

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

    validate(settings,settingsMeta);
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