#pragma once

#include <cassert>
#include <unistd.h>
#include <map>

#include "async_worker.h"
#include "chash.h"
#include "coding.h"
#include "cqueue.h"
#include "file.h"
#include "freq_ctrl.h"
#include "singleton.h"
#include "slice.h"
#include "random.h"
#include "rob.h"
#include "timer.h"

namespace cutils {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
using UniPtr = std::unique_ptr<T>;

template<typename T>
using UniPtrVec = std::vector<std::unique_ptr<T>>;

void ParseCfgString(
        const char* cfgstr, std::map<std::string, std::string>& opts);

void InitCpuBinding(const char* cfgstr);
int CpuBinding(int beg, int end);
int CpuBinding(const char* type, char* msg = nullptr);
void GetCpuBindingMask(std::string* txt);

int GetDiskSize(const char* path, size_t& tot_size,
                size_t& avail_size, size_t& free_size);
size_t GetMachineMemorySize();
size_t GetProcVmRSS(int pid = 0);
int GetPid();
int GetTid();

inline bool SleepWithStop(int sec, bool& stop) {
    for (int i = 0; i < sec && !stop; ++i) sleep(1);
    return stop;
}

inline bool SleepWithStop(int sec, int& stop) {
    for (int i = 0; i < sec && !stop; ++i) sleep(1);
    return stop;
}

template <typename F>
struct ScopeExit {
    ScopeExit(F f) : f(f) {}
    ~ScopeExit() { f(); }
    F f;
};

template <typename F>
ScopeExit<F> MakeScopeExit(F f) {
    return ScopeExit<F>(f);
};

} // namespace cutils

#define CUTILS_VARGS_(_10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N
#define CUTILS_VARGS(...) CUTILS_VARGS_(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define CUTILS_DO_STRING_JOIN2(arg1, arg2) arg1 ## arg2
#define CUTILS_STRING_JOIN2(arg1, arg2) CUTILS_DO_STRING_JOIN2(arg1, arg2)

#ifdef MMERR
#define hassert_1(cond)                                     \
{                                                           \
    bool bCond = cond;                                      \
    if (!bCond) {                                           \
        MMERR("[ASSERT]");                                  \
    }                                                       \
    assert(bCond);                                          \
}
#define hassert_n(cond, fmt, ...)                           \
{                                                           \
    bool bCond = cond;                                      \
    if (!bCond) {                                           \
        printf (fmt "\n", ##__VA_ARGS__);                   \
        MMERR("[ASSERT] " fmt, ##__VA_ARGS__);              \
    }                                                       \
    assert(bCond);                                          \
}
#else
#define hassert_1(cond) assert(cond);
#define hassert_n(cond, fmt, ...)                           \
{                                                           \
    bool bCond = cond;                                      \
    if (!bCond) {                                           \
        printf (fmt "\n", ##__VA_ARGS__);                   \
    }                                                       \
    assert(bCond);                                          \
}
#endif

#define hassert_2 hassert_n
#define hassert_3 hassert_n
#define hassert_4 hassert_n
#define hassert_5 hassert_n
#define hassert_6 hassert_n
#define hassert_7 hassert_n
#define hassert_8 hassert_n
#define hassert_9 hassert_n
#define hassert_10 hassert_n
#ifdef hassert
#undef hassert
#endif

#define hassert(...) CUTILS_STRING_JOIN2(hassert_, CUTILS_VARGS(__VA_ARGS__))(__VA_ARGS__)
#define cdefer(code) auto CUTILS_STRING_JOIN2(_cdefer_, __LINE__) = cutils::MakeScopeExit([&](){code;})
#define CDEFER(code) auto CUTILS_STRING_JOIN2(_cdefer_, __LINE__) = cutils::MakeScopeExit([&](){code;})

