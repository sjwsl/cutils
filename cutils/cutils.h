#pragma once

#include "chash.h"
#include "singleton.h"
#include "slice.h"
#include "random.h"
#include "rob.h"
#include "timer.h"

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
using UniPtr = std::unique_ptr<T>;

template<typename T>
using UniPtrVec = std::vector<std::unique_ptr<T>>;