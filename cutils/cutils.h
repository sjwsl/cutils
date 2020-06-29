#pragma once

#include "chash.h"
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

template<typename T>
void PrintVector(const T& t) {
    std::copy(t.cbegin(), t.cend(), std::ostream_iterator<typename T::value_type>(std::cout, ", "));
    std::cout << "\n";
}

void SleepForMs(size_t ms) {
    timeval timeout;
    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = ms % 1000 * 1000;
    ::select(0, NULL, NULL, NULL, &timeout);
}

#define AppendColorStr(result, color, fmt, ...) { \
    char sch[1024]; \
    if (strcmp((color), "red") == 0) { \
        sprintf(sch, "\033[31m" fmt "\033[0m", ##__VA_ARGS__); \
    } else if (strcmp((color), "green") == 0) { \
        sprintf(sch, "\033[32m" fmt "\033[0m", ##__VA_ARGS__); \
    } else if (strcmp((color), "yellow") == 0) { \
        sprintf(sch, "\033[33m" fmt "\033[0m", ##__VA_ARGS__); \
    } else { \
        sprintf(sch, fmt, ##__VA_ARGS__); \
    } \
    result.append(sch); \
}

} // namespace cutils