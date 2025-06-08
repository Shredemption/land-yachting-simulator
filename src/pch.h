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

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

// OpenGL & GLFW
#ifndef __glad_h_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif

// My project
#include "animation/animation.hpp"
#include "camera/camera.hpp"
#include "event_handler/event_handler.hpp"
#include "file_manager/file_manager.hpp"
#include "framebuffer/framebuffer_util.hpp"
#include "model/model.hpp"
#include "model/model_util.hpp"
#include "model/bone.h"
#include "render/render.hpp"
#include "scene/scene.hpp"
#include "scene_manager/scene_manager.hpp"
#include "settings_manager/settings_manager.hpp"
#include "shader/shader_util.hpp"
#include "shader/shaderID.h"
#include "texture_manager/texture_manager.hpp"
#include "thread_manager/thread_manager.hpp"
#include "ui_manager/ui_manager.hpp"

// Utilites
#include "easing_functions.h"