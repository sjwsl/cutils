#include "freq_ctrl.h"

#include <poll.h>
#include "timer.h"

namespace cutils {

void FreqCtrl::Init(const char* name, size_t max_sz) {
    name_ = name;
    tot_sz_ = 0;
    max_sz_ = max_sz;

    if (bg_ != nullptr) return;
    bg_ = cutils::AsyncWorker::Make(&FreqCtrl::BgWorker, this);
}

bool FreqCtrl::ShouldGo(size_t sz) {
    if (tot_sz_ > max_sz_) return false;

    if (sz == 1) {
        size_t now_sz = (++tot_sz_);
        if (now_sz > max_sz_) return false;

    } else {
        for (int i = 0; i < 16; ++i) {
            size_t now_sz = tot_sz_.load();
            if (now_sz + sz > max_sz_) return false;
            if (tot_sz_.compare_exchange_weak(now_sz, now_sz + sz)) {
                return true;
            }
        }
        return false;
    }

    return true;
}

void FreqCtrl::SyncGo(size_t sz) {
    for (int i = 0; i < 100; ++i) {
        size_t now_sz = tot_sz_.load();
        if (now_sz + sz <= max_sz_ && tot_sz_.compare_exchange_weak(
                    now_sz, now_sz + sz)) {
            return;
        }

        int delay = rand() % (now_sz + sz > max_sz_ ? 20 : 5);
        poll(nullptr, 0, delay);
    }

    tot_sz_ += sz;
}

void FreqCtrl::BgWorker(bool& stop) {
    cutils::SetThreadTitle("fct_%s", name_.c_str());
    while (!stop) {
        poll(nullptr, 0, 100);
        tot_sz_ = 0;
    }
}



FreqCtrlSingle::FreqCtrlSingle(size_t speed_per_sencod) : 
    avaliable_time_(0), speed_per_sencod_(speed_per_sencod) {}

void FreqCtrlSingle::Go(size_t sz) {
    if (sz == 0) return;

    uint64_t cur_ts = GetTimeStampInMS();
    if (cur_ts < avaliable_time_) {
        poll(nullptr, 0, avaliable_time_ - cur_ts);
    }

    uint64_t cost_time_in_ms = sz * 1000. / speed_per_sencod_;
    avaliable_time_ = GetTimeStampInMS() + cost_time_in_ms;
}

} // namespace cutils

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL$ $Id$ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

