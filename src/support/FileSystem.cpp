#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

namespace utils {
void ensure_directory_exists(const std::string& path) {
  fs::path dir_path(path);
  if (!fs::exists(dir_path)) {
    if (fs::create_directories(dir_path)) {
      std::cout << "Directory created: " << path << std::endl;
    } else {
      std::cerr << "Failed to create directory: " << path << std::endl;
    }
  } else {
    std::cout << "Directory already exists: " << path << std::endl;
  }
}
/**
 * 00_main.sy -> 00_main
 */
std::string preName(const std::string& filePath) {
  size_t lastSlashPos = filePath.find_last_of("/\\");
  if (lastSlashPos == std::string::npos) {
    lastSlashPos = -1;  // 如果没有找到 '/', 则从字符串开头开始
  }

  // 找到最后一个 '.' 的位置
  size_t lastDotPos = filePath.find_last_of('.');
  if (lastDotPos == std::string::npos || lastDotPos < lastSlashPos) {
    lastDotPos = filePath.size();  // 如果没有找到 '.', 则到字符串末尾
  }

  // 提取 '/' 和 '.' 之间的子字符串
  return filePath.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
};

}  // namespace utils