#include "scene_manager/scene_manager.h"

#include <jsoncons/json.hpp>

#include <filesystem>
#include <fstream>

#include "physics/physics.h"
#include "animation/animation.h"
#include "render/render.h"
#include "shader/shader.h"
#include "camera/camera.h"

Scene *SceneManager::currentScene = nullptr;
std::atomic<bool> SceneManager::isLoading = false;
std::thread SceneManager::loadingThread;
GLFWwindow *SceneManager::mainWindow = nullptr;

std::map<std::string, std::string> SceneManager::sceneMap;
std::string sceneMapPath = "resources/scenes.json";
bool SceneManager::onTitleScreen = false;

void SceneManager::load(const std::string &sceneName)
{
    unload();

    if (sceneName == "title")
    {
        onTitleScreen = true;
    }

    currentScene = new Scene(sceneMap[sceneName], sceneName);

    Camera::reset();
    Physics::setup(*currentScene);
}

void SceneManager::loadDetached(const std::string &sceneName)
{
    if (isLoading)
    {
        return;
    }

    isLoading.store(true);

    unload();

    if (sceneName == "title")
    {
        onTitleScreen = true;
    }

    loadingThread = std::thread([sceneName]()
                                {

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        GLFWwindow* loadContext = glfwCreateWindow(1, 1, "LoadingContext", NULL, glfwGetCurrentContext());
        if (!loadContext)
        {
            std::cerr << "Failed to create OpenGL context for loading thread!" << std::endl;
            isLoading.store(false);
            return;
        }

        glfwMakeContextCurrent(loadContext);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD in loading thread!" << std::endl;
            isLoading.store(false);
            return;
        }

        currentScene = new Scene(sceneMap[sceneName], sceneName);

        Camera::reset();
        Physics::setup(*currentScene);

        glFinish();

        glfwDestroyWindow(loadContext);      

        isLoading.store(false); });

    loadingThread.detach();
}

void SceneManager::update()
{
    Physics::update(*currentScene);
    Animation::updateBones(*currentScene);
}

void SceneManager::render()
{
    Render::render(*currentScene);
}

void SceneManager::unload()
{
    if (currentScene)
    {
        delete currentScene;
        currentScene = nullptr;
    }

    onTitleScreen = false;

    Shader::loadedShaders.clear();
    Shader::waterLoaded = false;
}

void SceneManager::loadSceneMap()
{
    const std::string path = "../" + sceneMapPath;
    // Check if the file exists
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("File not found: " + path);
    }

    // Open the file
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + path);
    }

    // Parse the JSON
    jsoncons::json j;
    try
    {
        j = jsoncons::json::parse(file);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }

    // Create a map from the parsed JSON
    for (const auto &kv : j["scenes"].object_range())
    {
        sceneMap[kv.key()] = kv.value().as<std::string>();
    }
}

void SceneManager::renderLoading()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Render::renderText("Loading...", 0.1f, 0.8f, 1, glm::vec3(1.0f, 1.0f, 1.0f));
}