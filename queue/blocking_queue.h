#pragma once

#include <mutex>
#include <condition_variable>
#include <iostream>
#include <chrono>
#include <queue>
#include <stdexcept>
#include <cassert>

#include "conlib/common/error.h"

namespace conlib {

template<typename T>
class BlockingQueue {
public:
    BlockingQueue();

    explicit BlockingQueue(size_t max_size);

    ~BlockingQueue();

    ErrorCode offer(T &&t, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    ErrorCode offer_util(T &&t, std::chrono::system_clock::time_point timeout_point);

    ErrorCode take(T &res, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    ErrorCode take_util(T &res, std::chrono::system_clock::time_point timeout_point);

    size_t len();

    void close();

private:
    size_t m_max_size = 0;
    std::deque<T> m_queue = {};

    bool m_closed = false;
    std::mutex m_mutex = {};
    std::condition_variable m_not_empty = {};
    std::condition_variable m_not_full = {};
};

template<typename T>
BlockingQueue<T>::BlockingQueue()
        : BlockingQueue<T>::BlockingQueue(std::numeric_limits<std::size_t>::max()) {
}

template<typename T>
BlockingQueue<T>::BlockingQueue(size_t max_size) {
    assert(max_size > 0);

    m_max_size = max_size;
}

template<typename T>
BlockingQueue<T>::~BlockingQueue() = default;

template<typename T>
ErrorCode BlockingQueue<T>::offer(T &&t, std::chrono::milliseconds timeout) {
    if (timeout.count() == 0) {
        return offer_util(std::forward<T>(t), std::chrono::system_clock::time_point());
    } else {
        return offer_util(std::forward<T>(t), std::chrono::system_clock::now() + timeout);
    }
}

template<typename T>
ErrorCode BlockingQueue<T>::offer_util(T &&t, std::chrono::system_clock::time_point timeout_point) {
    std::unique_lock<std::mutex> lock(m_mutex);

    if (timeout_point > std::chrono::system_clock::now()) {
        while (len() >= m_max_size && !m_closed) {
            if (m_not_full.wait_until(lock, timeout_point) == std::cv_status::timeout) {
                return WaitTimeout;
            }
        }
    } else if (timeout_point.time_since_epoch().count() == 0) {
        while (len() >= m_max_size && !m_closed) {
            m_not_full.wait(lock);
        }
    } else {
        return IllegalTimePoint;
    }

    if (m_closed) {
        return QueueClosed;
    }

    m_queue.emplace_back(std::forward<T>(t));

    m_not_empty.notify_one();

    return OK;
}

template<typename T>
ErrorCode BlockingQueue<T>::take(T &res, std::chrono::milliseconds timeout) {
    if (timeout.count() == 0) {
        return take_util(res, std::chrono::system_clock::time_point());
    } else {
        return take_util(res, std::chrono::system_clock::now() + timeout);
    }
}

template<typename T>
ErrorCode BlockingQueue<T>::take_util(T &res, std::chrono::system_clock::time_point timeout_point) {
    std::unique_lock<std::mutex> lock(m_mutex);

    if (timeout_point > std::chrono::system_clock::now()) {
        while (len() <= 0 && !m_closed) {
            if (m_not_empty.wait_until(lock, timeout_point) == std::cv_status::timeout) {
                return WaitTimeout;
            }
        }
    } else if (timeout_point.time_since_epoch().count() == 0) {
        while (len() <= 0 && !m_closed) {
            m_not_empty.wait(lock);
        }
    } else {
        return IllegalTimePoint;
    }

    if (m_closed) {
        return QueueClosed;
    }

    res = std::move(m_queue.front());
    m_queue.pop_front();

    m_not_full.notify_one();

    return OK;
}

template<typename T>
size_t BlockingQueue<T>::len() {
    return m_queue.size();
}

template<typename T>
void BlockingQueue<T>::close() {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_closed) {
        return;
    }

    m_closed = true;
    m_not_full.notify_all();
    m_not_empty.notify_all();
}

} // namespace conlib
