#include "file_manager.h"

#include <iostream>
#include <filesystem>
#include <fstream>

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