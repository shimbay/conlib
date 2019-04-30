#pragma once

#include <pthread.h>
#include <cassert>

namespace conlib {

enum LockType {
    READ,
    WRITE
};

typedef pthread_rwlock_t RWMutex;

class ReadLock {
public:
    explicit ReadLock(RWMutex &mutex) {
        m_mutex = &mutex;
        pthread_rwlock_rdlock(m_mutex);
    }

    ~ReadLock() {
        pthread_rwlock_unlock(m_mutex);
    }

private:
    RWMutex *m_mutex;
};

class WriteLock {
public:
    explicit WriteLock(RWMutex &mutex) {
        m_mutex = &mutex;
        pthread_rwlock_wrlock(m_mutex);
    }

    ~WriteLock() {
        pthread_rwlock_unlock(m_mutex);
    }

private:
    RWMutex *m_mutex;
};

} // namespace conlib
