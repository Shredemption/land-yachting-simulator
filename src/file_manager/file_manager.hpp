#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <string>

namespace FileManager
{
    std::string read(const std::string &filename);
    std::string getPath(const std::string &path);
};

#endif