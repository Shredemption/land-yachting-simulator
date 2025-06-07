#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <string>

#include "settings_manager/settings.h"

namespace SettingsManager
{
    inline Settings settings;
    inline std::string settingsFile = "settings.json";

    void load();
    void save();
}

#endif