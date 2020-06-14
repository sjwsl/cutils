#pragma once

#include <cstdint>
#include <cstring>

namespace cutils {

template<typename T>
inline bool HasFlag(T mask, T flag) {  return (mask & flag) > 0; }

template<typename T>
inline void AddFlag(T& mask, T flag) { mask |= flag; }

template<typename T>
inline void ClearFlag(T& mask, T flag) { mask &= (~flag); }

template<typename T>
inline size_t Encoding(char* buf, T x) { 
    size_t sz = sizeof(x);
    memcpy(buf, (char*)&x, sz);
    return sz;
}

template<typename T>
inline size_t Decoding(const char* buf, T& x) { 
    size_t sz = sizeof(x);
    memcpy((char*)&x, buf, sz);
    return sz;
}


inline uint64_t U64(uint32_t hi, uint32_t lo) { 
    return ((((uint64_t)hi) << 32) | lo); 
}

inline uint32_t U64HI(uint64_t x) { 
    return (x >> 32); 
}

inline uint32_t U64LO(uint64_t x) { 
    return x;
}

inline uint32_t U32(uint16_t hi, uint16_t lo) { 
    return ((((uint32_t)hi) << 16) | lo); 
}

inline uint16_t U32HI(uint32_t x) { 
    return (x >> 16); 
}

inline uint16_t U32LO(uint32_t x) { 
    return x;
}

} // namespace cutils
