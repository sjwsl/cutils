#include "file.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cutils {

std::string ConcatePath(const std::string& path, const std::string& tail) {
    if (path.empty() || path.back() == '/') {
        return path + tail;
    }
    return path + "/" + tail;
}

std::string GetBaseName(const std::string& fn) {
    size_t pos = fn.find_last_of('/');
    return fn.substr(pos == std::string::npos ? 0 : pos + 1);
}

int ScanFiles(
        const std::string& dir, 
        const std::string& pattern, 
        std::vector<std::string>& files) {
    DIR* pDir = NULL;
    if (!(pDir = opendir(dir.c_str()))) {
        return -1;
    }

    struct dirent* pDirEnt = nullptr;
    while (pDirEnt = readdir(pDir)) {
        const char* fn = pDirEnt->d_name;
        if (strcmp(fn, ".") == 0 || strcmp(fn, "..") == 0) continue;
        if (pattern.empty() || strstr(fn, pattern.c_str())) {
            files.push_back(fn);
        }
    }

    closedir(pDir);
    return 0;
}

int Access(const std::string& fn) {
    return access(fn.c_str(), F_OK);
}

int Touch(const std::string& fn) {
    if (Access(fn) == 0) {
        return 0;
    }

    int fd = open(fn.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd < 0) {
        return -errno;
    }

    close(fd);
    return 0;
}

size_t FileSize(const std::string& fn) {
    struct stat finfo = {0};
    int ret = stat(fn.c_str(), &finfo);
    if (0 != ret) {
        return 0;
    }
    return finfo.st_size;
}

int EmptyDirectory(const std::string& dir) {
    DIR * handle = opendir(dir.c_str());
    if (nullptr == handle) {
        return -__LINE__;
    }

    char buffer[1024] = {0};
    struct dirent * cursor = NULL;
    while ((cursor = readdir(handle)) != NULL) {
        if (cursor->d_name[0] == '.') {
            continue;
        }

        snprintf(buffer, sizeof(buffer), "%s/%s", dir.c_str(), cursor->d_name);
        struct stat fs;
        if (stat(buffer, &fs) < 0) {
            return -__LINE__;
        }

        if (S_ISREG(fs.st_mode)) {
            unlink(buffer);
        } else if (S_ISDIR(fs.st_mode)) {
            int ret = EmptyDirectory(buffer);
            if (0 != ret) {
                return ret;
            }
            ret = rmdir(buffer);
            if (0 != ret) {
                return -__LINE__;
            }
        } else {
            return -__LINE__;
        }
    }

    closedir(handle);
    return 0;
}

int DeleteFileOrDir(const std::string& fn, int recursive) {
    if (0 != access(fn.c_str(), F_OK)) {
        return 0;
    }

    struct stat info = {0};
    int ret = stat(fn.c_str(), &info);
    if (0 != ret) {
        return ret;
    }

    if (false == S_ISDIR(info.st_mode)) {
        // file
        return unlink(fn.c_str());
    } else {
        // dir
        if (recursive == 0) {
            return -1;
        }
        
        ret = EmptyDirectory(fn.c_str());
        if (ret != 0) {
            return ret;
        }
        return rmdir(fn.c_str());
    }
}

} // namespace cutils

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL$ $Id$ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

