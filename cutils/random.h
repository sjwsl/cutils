#pragma once

#include <string>
#include "slice.h"

namespace cutils {

// return in [0, 2^31)
int DeviceRand();
int GenFastRandSeed();
int FastRand(int seed);

class StrRand {
private:
    int max_len_;
    int num_;
    int seed_;
    std::string buf_; 

public:
    StrRand(int max_len, int num, int seed = 0);
    ~StrRand() {}

    Slice Next(int len);
    Slice Get(int len, uint32_t seed);
};

} // namespace cutils
