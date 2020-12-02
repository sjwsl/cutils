#pragma once
#include "async_worker.h"
#include "singleton.h"

namespace cutils {

class FreqCtrl : public cutils::Singleton<FreqCtrl> {
public:
    void Init(const char* name, size_t max_sz);

    bool ShouldGo(size_t sz = 1);
    void SyncGo(size_t sz = 1);

private:
    void BgWorker(bool& stop);

private:
    std::string name_;
    size_t max_sz_ = 0;
    std::atomic<size_t> tot_sz_;
    std::unique_ptr<cutils::AsyncWorker> bg_ = nullptr;
};

class FreqCtrlSingle {
public:
    FreqCtrlSingle(size_t speed_per_sencod);
    void Go(size_t sz);

private:
    uint64_t avaliable_time_;
    size_t speed_per_sencod_;
};

} // namespace cutils
