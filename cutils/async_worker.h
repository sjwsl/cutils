#pragma once

#include <future>
#include <cassert>
#include <memory>

#include "cqueue.h"
#include "singleton.h"
#include "timer.h"

namespace cutils {

class AsyncWorker {
public:
    template <typename WorkerType, typename ...Args>
    static std::unique_ptr<AsyncWorker> Make(WorkerType func, Args&&... args) {
        return std::move(std::unique_ptr<AsyncWorker>(
                    new AsyncWorker(func, std::forward<Args>(args)...)));
    }

    template <typename WorkerType, typename ...Args>
    AsyncWorker(WorkerType func, Args... args) 
        : stop_(false) 
        , worker_(std::async(std::launch::async, 
                    func, std::forward<Args>(args)..., std::ref(stop_))) {
        assert(worker_.valid());
    }

    void Join() {
        if (worker_.valid()) {
            worker_.get();
        }
    }

    void Stop() {
        stop_ = true;
    }

    ~AsyncWorker() {
        stop_ = true;
        if (worker_.valid()) {
            worker_.get();
        }
    }

private:
    bool stop_;
    std::future<void> worker_;
};

using AsyncWorkerPtr = std::unique_ptr<AsyncWorker>;

int SetThreadTitle(const char* fmt ...);


struct AsyncSeqTaskCtx {
    std::atomic<int> seq_alloc;
    std::atomic<int> code;
    AsyncSeqTaskCtx() : seq_alloc(0), code(0) {}
};

using AsyncTask = std::function<void()>;
using WorkerInititalizer = std::function<void()>;
using AsyncSeqTask = std::function<void(int, std::atomic<int>&)>;

struct AsyncSeqTaskProfiler {
    int      concur;
    int      max_seq;
    uint32_t beg_ts;
    uint32_t end_ts;
    std::vector<uint32_t> worker_beg_ts;
    std::vector<uint32_t> worker_end_ts;
    std::vector<uint32_t> worker_handle;
    std::vector<uint32_t> task_runtime;
    
    std::string Format();
};

class AsyncWorkerPool {
private:
    int tot_worker_ = 0;
    BlockingCQueue<AsyncTask> queue_;
    std::atomic<int> active_worker_;
    std::vector<AsyncWorkerPtr> workers_;

private:
    void WorkerRun(WorkerInititalizer initializer, bool& stop);

public:
    AsyncWorkerPool(int tot_worker, int queue_size = 1, 
            WorkerInititalizer initializer = nullptr);
    ~AsyncWorkerPool();

    int WorkerCount() { return tot_worker_; }
    int ActiveWorkerCount() { return active_worker_; }
    size_t QueuingTaskCount() { return queue_.Size(); }

    void AddTask(AsyncTask task);
    int RunSeqTaskAndWait(int concur, int max_seq, AsyncSeqTask seq_task, 
                          AsyncSeqTaskProfiler* profiler = nullptr);
};

using CEvent = std::function<void()>;
class CEventTick : public Singleton<CEventTick> {
private:
    std::vector<CEvent> events_1s_;
    std::vector<CEvent> events_10s_;
    std::vector<CEvent> events_60s_;
    AsyncWorkerPtr bg_ = nullptr;

private:
    void BGWorker(bool& stop);

public:
    void Start();
    void Stop();
    bool AddEventPer1s(CEvent event);
    bool AddEventPer10s(CEvent event);
    bool AddEventPer60s(CEvent event);
};

} // namespace cutils
