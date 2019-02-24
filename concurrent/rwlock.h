#pragma once

#include <pthread.h>
#include <cassert>

namespace conlib {

enum LockType {
    READ,
    WRITE
};

typedef pthread_rwlock_t RWMutex;

class RWLock {
public:
    explicit RWLock(RWMutex &mutex, LockType type) {
        m_mutex = &mutex;
        switch (type) {
            case READ: {
                pthread_rwlock_rdlock(m_mutex);
                break;
            }
            case WRITE: {
                pthread_rwlock_wrlock(m_mutex);
                break;
            }
            default: {
                assert(false && "Illegal Lock Type");
            }
        }
    }

    ~RWLock() {
        pthread_rwlock_unlock(m_mutex);
    }

private:
    RWMutex *m_mutex;
};

} // namespace conlib
