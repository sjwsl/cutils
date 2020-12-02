#include "random.h"

#include <random>

namespace cutils {

int DeviceRand() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0);
    return dis(gen);
}

int GenFastRandSeed() {
    int ret = DeviceRand();
    if (ret == 0 || ret == 2147483647) {
        return 20180917;
    }
    return ret;
}

int FastRand(int seed) {
    static const uint32_t M = 2147483647L;
    static const uint64_t A = 16807;
    uint64_t product = seed * A;
    uint32_t v = (uint32_t)((product >> 31) + (product & M));
    if (v > M) {
        v -= M;
    }
    return (int)v;
}

StrRand::StrRand(int max_len, int num, int seed) {
    assert(max_len > 0);
    assert(num > 0);
    max_len_ = max_len;
    num_ = num;
    seed_ = seed ? seed : DeviceRand();
    buf_.resize(num + max_len + num - 1);
    for (auto& ch : buf_) {
        ch = 'a' + seed_ % 26;
        seed_ = FastRand(seed_);
    }
}

Slice StrRand::Next(int len) {
    assert(len <= max_len_);
    int pos = seed_ % num_;
    seed_ = FastRand(seed_);
    return Slice(buf_.data() + pos, len);
}

Slice StrRand::Get(int len, uint32_t seed) {
    int pos = seed % num_;
    return Slice(buf_.data() + pos, len);
}

} // namespace cutils

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL$ $Id$ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

