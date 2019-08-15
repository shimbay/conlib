#pragma once

#include <cassert>
#include <thread>

#include "conlib/common/common.h"
#include "conlib/common/error.h"
#include "conlib/queue/blocking_queue.h"

namespace conlib {

template <typename T> class WorkerGroup {
public:
  explicit WorkerGroup(size_t worker_num);

  WorkerGroup(size_t worker_num, size_t queue_size);

  virtual ~WorkerGroup();

  ErrorCode submit(T &&t);

  size_t size();

protected:
  virtual void work(T &t) = 0;

private:
  size_t m_worker_num = 0;

  BlockingQueue<T> m_queue = {};
  std::vector<std::thread> m_workers = {};

  void _work();
};

template <typename T> WorkerGroup<T>::WorkerGroup(size_t worker_num) : m_worker_num(worker_num) {
  assert(m_worker_num > 0);

  for (size_t i = 0; i < m_worker_num; ++i) {
    m_workers.emplace_back(std::thread(&WorkerGroup::_work, this));
  }
}

template <typename T>
WorkerGroup<T>::WorkerGroup(size_t worker_num, size_t queue_size)
    : m_worker_num(worker_num), m_queue(queue_size) {
  assert(m_worker_num > 0);

  for (size_t i = 0; i < m_worker_num; ++i) {
    m_workers.emplace_back(std::thread(&WorkerGroup::_work, this));
  }
}

template <typename T> WorkerGroup<T>::~WorkerGroup() {
  for (auto &m_worker : m_workers) {
    m_worker.detach();
  }

  m_queue.close();
}

template <typename T> ErrorCode WorkerGroup<T>::submit(T &&t) {
  return m_queue.offer(std::forward<T>(t));
}

template <typename T> size_t WorkerGroup<T>::size() { return m_queue.len(); }

template <typename T> void WorkerGroup<T>::_work() {
  while (true) {
    T t;
    auto err = m_queue.take(t);
    if (err == QueueClosed) {
      return;
    }
    if (err != OK) {
      continue;
    }
    try {
      work(t);
    } catch (...) {
      handle_exception(std::current_exception(), "worker handle error");
    }
  }
}

} // namespace conlib
