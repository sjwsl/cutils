#pragma once

#include <cstdlib>

namespace cutils {

template<typename T>
inline T CHash(const char* data, size_t size, T seed = 271, T sum = 0) {
    const char* end = data + size;
    while (data < end) {
        sum = sum * seed + (*data++);
    }
    return sum;
}

template<typename T>
inline T CHash2(const char* data, size_t size, T seed = 271, T sum = 0) {
    const char* end = data + size;
    while (data < end) {
        T ch = *(uint8_t*)(data++);
        sum = sum * seed + 1 + ch;
    }
    return sum;
}

// knuth multiplicative hash
inline uint32_t UinHash(uint32_t uin) {
    return uin * 2654435761UL;
}

} // namespace cutils
