#pragma once

#include <glm/glm.hpp>

#include <string>
#include <optional>
#include <vector>

#include "physics/physics_defs.h"

struct ModelData;

class Physics
{
public:
    // Constructor
    Physics(const ModelData &model);

    void update(bool &controlled);
    void reset(const glm::mat4 &u_model);
    void savePrevState();
    void copyFrom(const Physics &other);

    BaseVariables base;
    std::optional<BodyVariables> bodyVariables;
    std::optional<SailVariables> sailVariables;
    std::optional<DrivingVariables> drivingVariables;
    std::optional<CollisionVariables> collisionVariables;

    bool applyGravity = false;

    std::vector<DebugForce> debugForces;

    void getVariablePointers(BodyVariables *&body, SailVariables *&sails, DrivingVariables *&driving)
    {
        body = bodyVariables ? &bodyVariables.value() : nullptr;
        sails = sailVariables ? &sailVariables.value() : nullptr;
        driving = drivingVariables ? &drivingVariables.value() : nullptr;
    }

private:
    void updateInputs(bool debug);
    void updateSail(bool debug);
    void updateBody(bool debug);
    void updateDriving(bool debug);
    void checkCollisions(bool debug);
};