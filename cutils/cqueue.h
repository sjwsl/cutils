#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <chrono>
#include <cassert>

namespace cutils {

template <typename EntryType>
class CQueue {
private:
    const size_t max_queue_size_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::unique_ptr<EntryType>> msg_;

public:
    explicit CQueue(size_t max_queue_size = 0) : 
        max_queue_size_(max_queue_size) {}

    void Push(std::unique_ptr<EntryType> item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            msg_.emplace(std::move(item));
            assert(nullptr != msg_.back());
            if (0 < max_queue_size_ && max_queue_size_ < msg_.size()) {
                msg_.pop();
            }
            assert(0 == max_queue_size_ || max_queue_size_ >= msg_.size());
        }
        cv_.notify_one();
    }

    void BatchPush(
            std::vector<std::unique_ptr<EntryType>>& vec_item, 
            size_t pop_batch_size = 1, 
            size_t pop_worker_cnt = 1) {
        if (true == vec_item.empty()) {
            return ;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& item : vec_item) {
                msg_.emplace(std::move(item));
                assert(nullptr != msg_.back());
            }
            for (size_t idx = 0; idx < vec_item.size(); ++idx) {
                if (0 == max_queue_size_ || max_queue_size_ >= msg_.size()) {
                    break;
                }

                msg_.pop();
            }
            assert(0 == max_queue_size_ || max_queue_size_ >= msg_.size());
        }

        assert(pop_batch_size > 0);
        size_t notify_cnt = 
            (vec_item.size() + pop_batch_size - 1) / pop_batch_size;
        assert(notify_cnt > 0);
        if (notify_cnt < pop_worker_cnt) {
            for (size_t idx = 0; idx < notify_cnt; ++idx) {
                cv_.notify_one();
            }
        } else {
            cv_.notify_all();
        }
    }

    std::unique_ptr<EntryType> Pop() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (msg_.empty()) {
                cv_.wait(lock, [&]() {
                        return !msg_.empty();
                        });
            }

            assert(false == msg_.empty());
            auto item = move(msg_.front());
            msg_.pop();
            return item;
        }
    }

    std::unique_ptr<EntryType> Pop(std::chrono::microseconds timeout) {
        auto time_point = std::chrono::system_clock::now() + timeout;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (false == 
                    cv_.wait_until(lock, time_point, 
                        [&]() {
                        return !msg_.empty();
                        })) {
                // timeout
                return nullptr;
            }

            assert(false == msg_.empty());
            auto item = move(msg_.front());
            msg_.pop();
            return item;
        }
    }

    std::vector<std::unique_ptr<EntryType>> BatchPop(
            size_t iMaxBatchSize, std::chrono::microseconds timeout) {
        auto time_point = std::chrono::system_clock::now() + timeout;
        std::vector<std::unique_ptr<EntryType>> vec;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (false == cv_.wait_until(lock, time_point, 
                        [&]() {
                        return !msg_.empty();
                        })) {
                return vec;
            }

            if (msg_.empty()) {
                return vec;
            }

            while (false == msg_.empty() && vec.size() < iMaxBatchSize) {
                assert(nullptr != msg_.front());
                auto item = std::move(msg_.front());
                msg_.pop();
                assert(nullptr != item);
                vec.push_back(std::move(item));
                assert(nullptr != vec.back());
            }
        }

        return std::move(vec);
    }

    std::vector<std::unique_ptr<EntryType>> BatchPop(size_t iMaxBatchSize) {
        std::vector<std::unique_ptr<EntryType>> vec;
        
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (msg_.empty()) {
                cv_.wait(lock, [&]() {
                        return !msg_.empty();
                        });
            }

            assert(false == msg_.empty());
            while (false == msg_.empty() && vec.size() < iMaxBatchSize) {
                assert(nullptr != msg_.front());
                auto item = std::move(msg_.front());
                msg_.pop();
                assert(nullptr != item);
                vec.push_back(std::move(item));
                assert(nullptr != vec.back());
            }

        }

        assert(false == vec.empty());
        return std::move(vec);
    }

    int BatchPopNoWait(
            size_t iMaxBatchSize, 
            std::vector<std::unique_ptr<EntryType>>& vec) {
        vec.clear();

        std::lock_guard<std::mutex> lock(mutex_);
        if (msg_.empty()) {
            return 1;
        }

        assert(false == msg_.empty());
        while (false == msg_.empty() && vec.size() < iMaxBatchSize) {
            assert(nullptr != msg_.front());
            auto item = std::move(msg_.front());
            msg_.pop();
            assert(nullptr != item);
            vec.push_back(std::move(item));
            assert(nullptr != vec.back());
        }

        return 0;
    }

    size_t Size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return msg_.size();
    }
};



template <typename EntryType>
class BlockingCQueue {
private:
    size_t capacity_;
    std::mutex mutex_;
    std::deque<EntryType> queue_;
    std::condition_variable cv_in_;
    std::condition_variable cv_out_;

public:
    explicit BlockingCQueue(size_t capacity) : capacity_(capacity) {}

    size_t Capacity() {
        return capacity_;
    }

    size_t Size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool Empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
    }
    
    void Push(EntryType&& item) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (queue_.size() >= capacity_) {
                cv_out_.wait(lock);
            }
            assert(queue_.size() < capacity_);
            queue_.push_back(std::move(item));
            cv_in_.notify_one();
        }
    }

    void Push(const EntryType& item) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (queue_.size() >= capacity_) {
                cv_out_.wait(lock);
            }
            assert(queue_.size() < capacity_);
            queue_.push_back(item);
            cv_in_.notify_one();
        }
    }

    EntryType Pop() { 
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (queue_.empty()) {
                cv_in_.wait(lock);
            }
            assert(queue_.size() > 0);
            auto ret = std::move(queue_.front());
            queue_.pop_front();
            cv_out_.notify_one();
            return ret;
        }
        assert(0);
    }

    bool TryPush(EntryType&& item, int timeoutInMillisecond) {
        auto tp = std::chrono::system_clock::now() + 
            std::chrono::milliseconds(timeoutInMillisecond);
        {
            std::unique_lock<std::mutex> lock(mutex_);
            bool ret = cv_out_.wait_until(lock, tp, [this]{
                    return queue_.size() < capacity_;});
            if (ret) {
                assert(queue_.size() < capacity_);
                queue_.push_back(std::move(item));
                cv_in_.notify_one();
            }
            return ret;
        }
    }

    bool TryPush(const EntryType& item, int timeoutInMillisecond) {
        auto tp = std::chrono::system_clock::now() + 
            std::chrono::milliseconds(timeoutInMillisecond);
        {
            std::unique_lock<std::mutex> lock(mutex_);
            bool ret = cv_out_.wait_until(lock, tp, [this]{
                    return queue_.size() < capacity_;});
            if (ret) {
                assert(queue_.size() < capacity_);
                queue_.push_back(item);
                cv_in_.notify_one();
            }
            return ret;
        }
    }

    bool TryPop(EntryType* item, int timeoutInMillisecond) {
        auto tp = std::chrono::system_clock::now() + 
            std::chrono::milliseconds(timeoutInMillisecond);
        {
            std::unique_lock<std::mutex> lock(mutex_);
            bool ret = cv_in_.wait_until(lock, tp, [this]{
                    return !queue_.empty();});
            if (ret) {
                assert(!queue_.empty());
                *item = std::move(queue_.front());
                queue_.pop_front();
                cv_out_.notify_one();
            }
            return ret;
        }
    }

    bool TryPop(EntryType* item, int maxCnt, int& iCnt,
                int timeoutInMillisecond) {
        if (maxCnt <= 0) return false;
        auto tp = std::chrono::system_clock::now() + 
            std::chrono::milliseconds(timeoutInMillisecond);
        {
            std::unique_lock<std::mutex> lock(mutex_);
            bool ret = cv_in_.wait_until(lock, tp, [this]{
                    return !queue_.empty();});
            if (ret) {
                assert(!queue_.empty());
                iCnt = 0;
                while (!queue_.empty() && iCnt < maxCnt) {
                    item[iCnt] = std::move(queue_.front());
                    queue_.pop_front();
                    ++iCnt;
                }

                if (iCnt <= 4) {
                    for (int i = 0; i < iCnt; ++i) {
                        cv_out_.notify_one();
                    }
                } else {
                    cv_out_.notify_all();
                }
            }
            return ret;
        }
    }
};


} // namespace cutils
