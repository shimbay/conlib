#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <chrono>
#include <cassert>
#include <queue>

#include "conlib/common/error.h"
#include "conlib/common/common.h"
#include "conlib/common/vector_tuple.h"

namespace conlib {

template<typename ...Ts>
class BatchWorkerGroup {
public:
    BatchWorkerGroup(size_t worker_num, size_t max_batch_size, std::chrono::milliseconds batch_timeout);

    virtual ~BatchWorkerGroup();

    ErrorCode submit(Ts &&... ts);

    size_t size();

protected:
    virtual void work(VectorTuple<Ts...> &data) = 0;

private:
    bool m_closed = false;
    bool m_hosting = false;
    size_t m_worker_num = 0;
    size_t m_max_batch_size = 0;
    std::chrono::milliseconds m_batch_timeout = {};

    std::mutex m_mutex = {};
    std::condition_variable m_wait = {};
    std::condition_variable m_batch = {};
    std::condition_variable m_empty_barrier = {};

    std::chrono::system_clock::time_point m_next_batch_timeout = {};

    size_t m_data_size = 0;
    std::deque<VectorTuple<Ts...>> m_buckets = {};

    std::vector<std::thread> m_workers = {};

    void _work();
};

template<typename ...Ts>
BatchWorkerGroup<Ts...>::BatchWorkerGroup(size_t worker_num, size_t max_batch_size, std::chrono::milliseconds batch_timeout)
        : m_worker_num(worker_num),
          m_max_batch_size(max_batch_size),
          m_batch_timeout(batch_timeout) {
    assert(m_worker_num >= 1);
    assert(m_max_batch_size >= 1);
    assert(m_batch_timeout.count() >= 0);

    for (size_t i = 0; i < m_worker_num; ++i) {
        m_workers.emplace_back(std::thread(&BatchWorkerGroup::_work, this));
    }
}

template<typename ...Ts>
BatchWorkerGroup<Ts...>::~BatchWorkerGroup() {
    for (auto &m_worker : m_workers) {
        m_worker.detach();
    }

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_closed = true;
        m_wait.notify_all();
        m_batch.notify_all();
        m_empty_barrier.notify_all();
    }
}

template<typename ...Ts>
ErrorCode BatchWorkerGroup<Ts...>::submit(Ts &&... ts) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_closed) {
        return QueueClosed;
    }

    if (m_buckets.size() == 0) {
        m_buckets.emplace_back(VectorTuple<Ts...>(m_max_batch_size));
    }

    emplace(m_buckets.back(), std::forward<Ts>(ts)...);

    if (m_data_size++ == 0) {
        m_next_batch_timeout = std::chrono::system_clock::now() + m_batch_timeout;
        m_empty_barrier.notify_one();
    }

    if (visit<0>(m_buckets.back()).size() >= m_max_batch_size) {
        m_buckets.emplace_back(VectorTuple<Ts...>(m_max_batch_size));
        m_batch.notify_one();
    }
    return OK;
}

template<typename... Ts>
size_t BatchWorkerGroup<Ts...>::size() {
    return m_data_size;
}

template<typename ...Ts>
void BatchWorkerGroup<Ts...>::_work() {
    while (!m_closed) {
        VectorTuple<Ts...> data;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (m_hosting) {
                m_wait.wait(lock);
                if (m_closed) {
                    return;
                }
            }
            m_hosting = true;

            if (m_data_size == 0) {
                m_empty_barrier.wait(lock);
                if (m_closed) {
                    return;
                }
            }

            if (m_next_batch_timeout > std::chrono::system_clock::now()
                    && visit<0>(m_buckets.front()).size() < m_max_batch_size) {
                m_batch.wait_until(lock, m_next_batch_timeout);
                if (m_closed) {
                    return;
                }
            }

            data = std::move(m_buckets.front());
            m_buckets.pop_front();
            m_data_size -= visit<0>(data).size();
            m_hosting = false;
            m_wait.notify_one();
        }

        try {
            if (visit<0>(data).size() != 0) {
                work(data);
            }
        } catch (...) {
            handle_exception(std::current_exception(), "batch worker handle error");
        }
    }
}

} // namespace conlib