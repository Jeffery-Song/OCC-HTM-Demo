#pragma once
#include <pthread.h>
#include <atomic>
#include <cstdio>

class RWSpinLock {
    // not its not a spinlock
    pthread_rwlock_t _lock;
    static const bool starve_w;
  public:
    RWSpinLock(bool starve_w) {
        if (!starve_w) {
            pthread_rwlockattr_t attr;
            pthread_rwlockattr_init(&attr);
            pthread_rwlockattr_setkind_np(&attr, 
            PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
            pthread_rwlock_init(&_lock, &attr);
        } else {
            pthread_rwlock_init(&_lock, nullptr);
        }
    }
    ~RWSpinLock() {
        pthread_rwlock_destroy(&_lock);
    }
    void rlock() {
        pthread_rwlock_rdlock(&_lock);
    }
    void wlock() {
        pthread_rwlock_wrlock(&_lock);
    }
    void unlock() {
        pthread_rwlock_unlock(&_lock);
    }
};
const bool RWSpinLock::starve_w = false;