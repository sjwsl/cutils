#pragma once

#include <poll.h>
#include <sys/time.h>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>

namespace cutils {

class StopWatch {
private:
	struct timeval m_startTime;
	struct timeval m_lastTime;
    std::vector<uint32_t> m_vec;

public:
	StopWatch(int capacity = 8) {
        m_vec.reserve(capacity);
        Clear();
    }

    inline void Clear() {
        gettimeofday(&m_startTime, nullptr);
        m_lastTime = m_startTime;
        m_vec.clear();
    }

	inline void Stop() {
        struct timeval t;
        gettimeofday(&t, nullptr);
        int diff = (t.tv_sec - m_lastTime.tv_sec) * 1000 + 
            (t.tv_usec - m_lastTime.tv_usec) / 1000;
        m_vec.push_back(diff);
        m_lastTime = t;
    }

    inline int Cost() {
        return (m_lastTime.tv_sec - m_startTime.tv_sec) * 1000 +
            (m_lastTime.tv_usec - m_startTime.tv_usec) / 1000;
    }

    inline std::string Format() {
        std::stringstream ss;
        ss << Cost();
        for (auto v : m_vec) {
            ss << " " << v;
        }
        return ss.str();
    }
};

class TimeDiff {
private:
	struct timeval m_startTime;
	struct timeval m_endTime;
public:
	TimeDiff() {gettimeofday(&m_startTime, NULL);}
	inline void Reset() {gettimeofday(&m_startTime, NULL);}
	inline void Stop() {gettimeofday(&m_endTime, NULL);}
    inline int ElapsedInSecond() {
        return (m_endTime.tv_sec - m_startTime.tv_sec);
    }
	inline int ElapsedInMillisecond() {
		return (m_endTime.tv_sec - m_startTime.tv_sec) * 1000 + 
			(m_endTime.tv_usec - m_startTime.tv_usec) / 1000;
    }
	inline int ElapsedInMicrosecond() {
		return (m_endTime.tv_sec - m_startTime.tv_sec) * 1000000 + 
			(m_endTime.tv_usec - m_startTime.tv_usec);
    }
    inline void StopAndWait(int interval_in_ms) {
        gettimeofday(&m_endTime, NULL);
        int rt_in_ms = ElapsedInMillisecond();
        if (rt_in_ms < interval_in_ms) {
            poll(nullptr, 0, interval_in_ms - rt_in_ms);
        }
    }
};

inline uint64_t GetTimeStampInMS() {
	struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

inline int GetLocalHour() {
    time_t t = time(0);
    struct tm buf;
    localtime_r(&t, &buf);
    return buf.tm_hour;
}

} // namespace cutils
