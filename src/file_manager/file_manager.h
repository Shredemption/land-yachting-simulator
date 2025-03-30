#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <cstdlib>

class FileManager
{
private:
    typedef std::string (*Builder)(const std::string &path);

public:
    static std::string read(const std::string &filename);

    static std::string getPath(const std::string &path)
    {
        static std::string (*pathbuilder)(std::string const &) = getPathBuilder();
        return (*pathbuilder)(path);
    }

private:
    static std::string const &getRoot()
    {
        static char const *envRoot = getenv("LOGL_ROOT_PATH");
        static char const *givenRoot = (envRoot != nullptr ? envRoot : "..");
        static std::string root = (givenRoot != nullptr ? givenRoot : "");
        return root;
    }

    static Builder getPathBuilder()
    {
        if (getRoot() != "")
            return &FileManager::getPathRelativeRoot;
        else
            return &FileManager::getPathRelativeBinary;
    }

    static std::string getPathRelativeRoot(const std::string &path)
    {
        return getRoot() + std::string("/") + path;
    }

    static std::string getPathRelativeBinary(const std::string &path)
    {
        return "../../../" + path;
    }
};

#endif