#include "async_worker.h"

#include <stdarg.h>
#include <poll.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "cutils.h"

namespace cutils {

int SetThreadTitle(const char* fmt ...) {
    char title[16] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(title, sizeof(title), fmt, ap);
    va_end(ap);
    return prctl(PR_SET_NAME, title);
}

std::string AsyncSeqTaskProfiler::Format() {
    std::string buf;
    buf.reserve(12800);
    std::stringstream ss(buf);
    ss << "AsyncSeqTaskProfiler"
       << " concur " << concur << " max_seq " << max_seq
       << " beg_ts " << beg_ts << " end_ts " << end_ts
       << " runtime " << end_ts - beg_ts;
    int n = std::min<int>(worker_beg_ts.size(), worker_end_ts.size());
    for (int i = 0; i < n; ++i) {
        ss << " worker_" << i << "_beg " << worker_beg_ts[i]
           << " worker_" << i << "_end " << worker_end_ts[i]
           << " worker_" << i << "_queue " << worker_beg_ts[i] - beg_ts
           << " worker_" << i << "_handle " << worker_handle[i];
    }
    ss << " task_runtime";
    for (size_t i = 0; i < task_runtime.size(); ++i) {
        ss << " " << task_runtime[i];
    }
    return ss.str();
}

void AsyncWorkerPool::WorkerRun(WorkerInititalizer initializer, bool& stop) {
    if (initializer) initializer();

    AsyncTask task;
    while (!stop) {
        bool got = queue_.TryPop(&task, 1000);
        if (stop) break;
        if (got) {
            active_worker_++;
            task();
            active_worker_--;
        }
    }
}

AsyncWorkerPool::AsyncWorkerPool(
        int tot_worker, int queue_size, WorkerInititalizer initializer) :
    tot_worker_(tot_worker), queue_(queue_size), active_worker_(0) {
    for (int i = 0; i < tot_worker; ++i) {
        workers_.emplace_back(std::move(AsyncWorker::Make(
                        &AsyncWorkerPool::WorkerRun, this, initializer)));
    }
}

AsyncWorkerPool::~AsyncWorkerPool() {
    for (auto& worker : workers_) {
        worker->Stop();
    }
    workers_.clear();
}

void AsyncWorkerPool::AddTask(AsyncTask task) {
    queue_.Push(std::move(task));
}

int AsyncWorkerPool::RunSeqTaskAndWait(
        int concur, int max_seq, AsyncSeqTask seq_task,
        AsyncSeqTaskProfiler* profiler) {
    int fds[2] = {0, 0};
    int ret = pipe(fds);
    assert(ret == 0);

    if (profiler) {
        profiler->concur = concur;
        profiler->max_seq = max_seq;
        profiler->beg_ts = time(0);
        profiler->worker_beg_ts.resize(concur);
        profiler->worker_end_ts.resize(concur);
        profiler->worker_handle.resize(concur);
        profiler->task_runtime.resize(max_seq);
    }

    std::atomic<int> seq_alloc(0), errcode(0), exited(0), wid(0);
    AsyncTask task = [&seq_alloc, &errcode, &exited, &wid, 
                      profiler, concur, max_seq, seq_task, fds]{
        int worker_id = (wid++);
        if (profiler) {
            profiler->worker_beg_ts[worker_id] = time(0);
        }
        while (seq_alloc < max_seq) {
            int seq = (seq_alloc++);
            if (seq >= max_seq) break;
            if (errcode == 0) {
                if (profiler) {
                    TimeDiff td;
                    seq_task(seq, errcode);
                    td.Stop();
                    profiler->task_runtime[seq] = td.ElapsedInSecond();
                    profiler->worker_handle[worker_id]++;
                } else {
                    seq_task(seq, errcode);
                }
            } else {
                // skip
            }
        }
        if (profiler) {
            profiler->worker_end_ts[worker_id] = time(0);
        }
        if ((++exited) == concur) {
            char c = 'o';
            int ret = write(fds[1], &c, sizeof(c));
            hassert(ret == sizeof(c), "%d %d %d", ret, errno, max_seq);
        }
    };

    for (int i = 0; i < concur; ++i) {
        queue_.Push(task);
    }

    {
        char c;
        int ret = read(fds[0], &c, sizeof(c));
        hassert(ret == sizeof(c), "%d %d", ret, errno);
    }
    close(fds[0]);
    close(fds[1]);

    if (profiler) {
        profiler->end_ts = time(0);
    }
    return errcode;
}

void CEventTick::BGWorker(bool& stop) {
    SetThreadTitle("cevent_bg");
    uint64_t last_10_ = GetTimeStampInMS();
    uint64_t last_60_ = GetTimeStampInMS();
    while (!stop) {
        uint64_t bt = GetTimeStampInMS();
        for (auto e : events_1s_) e();
        if (bt - last_10_ >= 10000) {
            for (auto e : events_10s_) e();
            last_10_ = GetTimeStampInMS();
        }
        if (bt - last_60_ >= 60000) {
            for (auto e : events_60s_) e();
            last_60_ = GetTimeStampInMS();
        }
        uint64_t rt = GetTimeStampInMS() - bt;
        if (rt < 1000) {
            poll(nullptr, 0, 1000 - rt);
        }
    }
}

void CEventTick::Start() {
    hassert(bg_ == nullptr);
    bg_ = AsyncWorker::Make(&CEventTick::BGWorker, this);
}

void CEventTick::Stop() {
    bg_ = nullptr;
}

bool CEventTick::AddEventPer1s(CEvent event) {
    if (bg_) return false;
    events_1s_.push_back(event);
    return true;
}

bool CEventTick::AddEventPer10s(CEvent event) {
    if (bg_) return false;
    events_10s_.push_back(event);
    return true;
}

bool CEventTick::AddEventPer60s(CEvent event) {
    if (bg_) return false;
    events_60s_.push_back(event);
    return true;
}

} // namespace cutils

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL$ $Id$ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

