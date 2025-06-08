#pragma once

#include "controller_manager/controller_manager_defs.h"

namespace ControllerManager
{
    inline bool controllerConnected;

    inline ControllerState state;

    void update();

    void buttonsMenu();
    void sticksMenu();

    void buttonsRunning();
    void sticksRunning();

    bool shouldTriggerNavigation(StickInput &stick);
};
