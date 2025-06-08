#include "file_manager.hpp"

#include "pch.h"

typedef std::string (*Builder)(const std::string &path);

std::string const &getRoot()
{
    static char const *envRoot = getenv("LOGL_ROOT_PATH");
    static char const *givenRoot = (envRoot != nullptr ? envRoot : "");
    static std::string root = (givenRoot != nullptr ? givenRoot : "");
    return root;
}

std::string getPathRelativeRoot(const std::string &path)
{
    return getRoot() + std::string("/") + path;
}

std::string getPathRelativeBinary(const std::string &path)
{
    return path;
}

Builder getPathBuilder()
{
    if (getRoot() != "")
        return &getPathRelativeRoot;
    else
        return &getPathRelativeBinary;
}

std::string FileManager::read(const std::string &filename)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::stringstream fileStream;
    try
    {
        file.open(filename.c_str());
        fileStream << file.rdbuf();
        file.close();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "FileManager: error reading file: " << filename << std::endl;
    }

    return fileStream.str();
}

std::string FileManager::getPath(const std::string &path)
{
    static std::string (*pathbuilder)(std::string const &) = getPathBuilder();
    return (*pathbuilder)(path);
}
