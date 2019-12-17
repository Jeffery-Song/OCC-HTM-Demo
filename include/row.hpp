#pragma once
#include "util.hpp"
#include <pthread.h>

template<typename PayloadT>
class Row { 
  private:
    uint64_t _version = 0;
    pthread_spinlock_t _lock;
  public:
    Row() {
      pthread_spin_init(&_lock, 0);
    }
    inline uint64_t get_version() const {return _version;}
    inline void inc_version() {_version++;}
    // template<typename RetType> 
    // RetType handle(std::function<RetType(PayloadT)> & func) {
    //     return func(payload);
    // }
    PayloadT payload;
    inline void lock() {
      pthread_spin_lock(&_lock);
    }
    inline void unlock() {
      pthread_spin_unlock(&_lock);
    }
};