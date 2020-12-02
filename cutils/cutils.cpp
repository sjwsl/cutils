#include "cutils.h"
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/vfs.h>

namespace cutils {

void ParseCfgString(
        const char* cfgstr, std::map<std::string, std::string>& opts) {
    opts.clear();
    size_t len = strlen(cfgstr), beg = 0;
    for (size_t i = 0; i <= len; ++i) {
        if (i == len || cfgstr[i] == ';') {
            if (i != beg) {
                for (size_t j = beg; j < i; ++j) {
                    if (cfgstr[j] == '=') {
                        std::string key(cfgstr + beg, j - beg);
                        std::string value(cfgstr + j + 1, i - j - 1);
                        opts[key] = value;
                    }
                }
            }
            beg = i + 1;
        }
    }
}

static std::map<std::string, std::pair<int, int>> gCpuBindingMap;
void InitCpuBinding(const char* cfgstr) {
    std::map<std::string, std::string> opts;
    ParseCfgString(cfgstr, opts);
    for (auto& item : opts) {
        int x = 0, y = 0;
        if (sscanf(item.second.c_str(), "%d,%d", &x, &y) == 2) {
            gCpuBindingMap[item.first] = {x, y};
        }
    }
}

int CpuBinding(int beg, int end) {
    int cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    if (beg < 0) beg += cpu_num;
    if (end < 0) end += cpu_num;
    if (beg > end) return -1;
    if (beg < 0 || beg >= cpu_num || end < 0 || end >= cpu_num) return -2;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = beg; i <= end; ++i) {
        CPU_SET(i, &mask);
    }
    sched_setaffinity(0, sizeof(mask), &mask);
    return 0;
}

int CpuBinding(const char* type, char* msg) {
    auto iter = gCpuBindingMap.find(type);
    if (iter == gCpuBindingMap.end()) {
        if (msg) {
            sprintf(msg, "[CpuBinding] type %s not found", type);
        }
        return -1;
    }
    int x = iter->second.first;
    int y = iter->second.second;
    int ret = CpuBinding(x, y);
    if (msg) {
        sprintf(msg, "[CpuBinding] type %s %d -> %d return %d", 
                type, x, y, ret);
    }
    return ret;
}

void GetCpuBindingMask(std::string* txt) {
    if (txt == nullptr) return;
    int cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    txt->resize(0);
    txt->resize(cpu_num, 'o');

    cpu_set_t mask;
    CPU_ZERO(&mask);
    sched_getaffinity(0, sizeof(mask), &mask);
    for (int i = 0; i < cpu_num; ++i) {
        if (CPU_ISSET(i, &mask)) {
            txt->at(i) = 'x';
        }
    }
}

int GetDiskSize(const char* path, size_t& tot_size,
                size_t& avail_size, size_t& free_size) {
    struct statfs info;
    int ret = statfs(path, &info);
    if (ret != 0) {
        return ret;
    }

    tot_size = info.f_blocks * info.f_bsize;
    free_size = info.f_bfree * info.f_bsize;
    avail_size = info.f_bavail * info.f_bsize;
    return 0;
}

size_t GetMachineMemorySize() {
    FILE *fp = fopen("/proc/meminfo","r");
    if (fp == nullptr) return 0;

    char buf[4096] = {0};
    fread(buf,1,sizeof(buf),fp);
    fclose(fp);

    char *lp = strstr(buf,"MemTotal");
    if (lp == nullptr) return 0;

    lp += strlen("MemTotal");
    while(*lp == ' ' || *lp == '\t' || *lp == ':') ++lp;

    size_t x = strtoull(lp, NULL, 10);
    return x * 1024;
}

size_t GetProcVmRSS(int pid) {
    char statm_path[128] = "/proc/self/statm";
    if (pid != 0) {
        sprintf(statm_path, "/proc/%d/statm", pid);
    }
    FILE* fp = NULL;
    if ((fp = fopen(statm_path, "r")) == nullptr) {
        return 0;
    }
    cdefer(fclose(fp));
    uint64_t vm_size = 0, vm_rss = 0;
    if (fscanf(fp, "%lu %lu", &vm_size, &vm_rss) != 2) {
        return 0;
    }
    return vm_rss * (size_t)sysconf( _SC_PAGESIZE);
}

int GetPid() {
    return getpid();
}

int GetTid() {
    return syscall(__NR_gettid);
}

} // namespace cutils

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL: http://scm-gy.tencent.com/gzrd/gzrd_mail_rep/QQMailcore_proj/trunk/platform/cutils/cutils.cpp $ $Id: cutils.cpp 3029578 2019-04-12 04:06:27Z flashlin $ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

