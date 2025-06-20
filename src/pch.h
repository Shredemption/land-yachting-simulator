#pragma once

// Standard Libraries
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include <optional>
#include <chrono>
#include <thread>
#include <future>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// OpenGL & GLFW
#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

// My project
#include "animation/animation.hpp"
#include "camera/camera.hpp"
#include "controller_manager/controller_manager.hpp"
#include "file_manager/file_manager.hpp"
#include "framebuffer/framebuffer_util.hpp"
#include "input_manager/input_manager.hpp"
#include "model/bone.h"
#include "model/model.hpp"
#include "model/model_util.hpp"
#include "physics/physics_util.hpp"
#include "render/render.hpp"
#include "scene/scene.hpp"
#include "scene_manager/scene_manager.hpp"
#include "settings_manager/settings_manager.hpp"
#include "shader/shaderID.h"
#include "shader/shader_util.hpp"
#include "texture_manager/texture_manager.hpp"
#include "thread_manager/thread_manager.hpp"
#include "time_manager/time_manager.hpp"
#include "ui_manager/ui_manager.hpp"
#include "window_manager/window_manager.hpp"

// Utilites
#include "easing_functions.h"