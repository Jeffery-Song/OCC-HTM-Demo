#pragma once
#include "util.hpp"
#include <pthread.h>

template<typename PayloadT>
class Row { 
  private:
    uint64_t _version = 0;
#ifndef USE_RTM_TXN
    pthread_spinlock_t _lock;
#endif
  public:
    PayloadT payload;

    Row() {
#ifndef USE_RTM_TXN
      pthread_spin_init(&_lock, 0);
#endif
    }
    inline uint64_t get_version() const {return _version;}
    inline void set_version(const uint64_t v) {_version = v;}
#ifndef USE_RTM_TXN
    inline void lock() {
      pthread_spin_lock(&_lock);
    }
#endif

#ifndef USE_RTM_TXN
    inline void unlock() {
      pthread_spin_unlock(&_lock);
    }
#endif
};