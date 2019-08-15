#pragma once

#include <cassert>
#include <pthread.h>

namespace conlib {

class RWMutex {
  friend class ReadLock;
  friend class WriteLock;

public:
  RWMutex() { pthread_rwlock_init(&m_mutex, nullptr); }

private:
  pthread_rwlock_t m_mutex{};
};

class ReadLock {
public:
  explicit ReadLock(RWMutex &mutex) {
    m_mutex = &mutex;
    pthread_rwlock_rdlock(&m_mutex->m_mutex);
  }

  ~ReadLock() { pthread_rwlock_unlock(&m_mutex->m_mutex); }

private:
  RWMutex *m_mutex;
};

class WriteLock {
public:
  explicit WriteLock(RWMutex &mutex) {
    m_mutex = &mutex;
    pthread_rwlock_wrlock(&m_mutex->m_mutex);
  }

  ~WriteLock() { pthread_rwlock_unlock(&m_mutex->m_mutex); }

private:
  RWMutex *m_mutex;
};

} // namespace conlib
