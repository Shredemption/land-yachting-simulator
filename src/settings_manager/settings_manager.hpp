#pragma once

#include <string>

#include "settings_manager/settings.h"

namespace SettingsManager
{
    inline SettingsStruct settings;
    inline LimitsStruct limits;
    inline std::string settingsFile = "settings.json";

    void load();
    void save();
}