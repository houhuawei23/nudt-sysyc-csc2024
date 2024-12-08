#pragma once
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

namespace utils {
void ensure_directory_exists(const std::string& path);

std::string preName(const std::string& filePath);

}  // namespace utils