#pragma once

#include <string>

namespace FileManager
{
    std::string read(const std::string &filename);
    std::string getPath(const std::string &path);
};