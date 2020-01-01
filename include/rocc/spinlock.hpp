#ifndef LEVELDB_SPINLOCK_H
#define LEVELDB_SPINLOCK_H

#include <stdint.h>
#include <immintrin.h>
#include <pthread.h>


#define barrier() asm volatile("": : :"memory")
#define cpu_relax() asm volatile("pause\n": : :"memory")

/* The counter should be initialized to be 0. */
class SpinLock  {
  private:
    pthread_spinlock_t lock;
    bool is_locked;
  public:

    SpinLock() : is_locked(false) { pthread_spin_init(&lock, 0);}
    
    inline void Lock() {
        pthread_spin_lock(&lock);
        is_locked = true;
    }

    inline void Unlock() {
        is_locked = false;
        pthread_spin_unlock(&lock);
    }

    inline uint16_t IsLocked(){return is_locked;}


};

#endif /* _RWLOCK_H */
