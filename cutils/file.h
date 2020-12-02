#pragma once

#include <string>
#include <vector>

namespace cutils {

std::string ConcatePath(const std::string& path, const std::string& tail);
std::string GetBaseName(const std::string& fn);
int ScanFiles(
        const std::string& dir, 
        const std::string& pattern, 
        std::vector<std::string>& files);
int Access(const std::string& fn);
int Touch(const std::string& fn);
size_t FileSize(const std::string& fn);
int EmptyDirectory(const std::string& dir);
int DeleteFileOrDir(const std::string& fn, int recursive);

} // namespace cutils
