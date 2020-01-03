#pragma once
#include "row.hpp"
#include "util.hpp"
#include "row.hpp"
#include "row_pool.hpp"
#include <vector>
#include <unordered_map>
#include <boost/pool/pool.hpp>

#ifdef USE_RTM_STORE
#include "rocc/rocc_htm.hpp"
#else
#include "rw_spin_lock.hpp"
#endif

template<typename PayloadT>
class ConcurrentHashMap {
  public:
    static thread_local uint64_t rtm_use_lock;
    static thread_local uint64_t rtm_retry;
  private:
#ifdef USE_RTM_STORE
    SpinLock _store_lock;
#else
    mutable RWSpinLock _lock;
#endif
    std::unordered_map<KeyType, void*> _db;
    RowPool<PayloadT> row_pool;
  public:
#ifdef USE_RTM_STORE
    ConcurrentHashMap() : row_pool() {}
#else
    ConcurrentHashMap() : _lock(false), row_pool() {}
#endif
    void* Read_or_Insert(const KeyType key) {
#ifdef USE_RTM_STORE
        // fprintf(stderr, "read or insert a rtm store\n");
        void * ret = nullptr;
        {
            RTMScope rtm(rtm_retry, rtm_use_lock, &_store_lock);
            {
                auto iter = _db.find(key);
                if (iter != _db.end()) {
                    ret = iter->second;
                } else {
                    ret = row_pool.malloc();
                    _db[key] = ret;
                }
            }
        }
        return ret;
#else
        _lock.rlock();
        auto iter = _db.find(key);
        if (iter != _db.end()) {
            void* r = iter->second;
            _lock.unlock();
            return r;
        }
        _lock.unlock();
        _lock.wlock();
        iter = _db.find(key);
        if (iter != _db.end()) {
            void* r = iter->second;
            _lock.unlock();
            return r;
        }
        void* ptr = row_pool.malloc();
        _db[key] = ptr;
        _lock.unlock();
        return ptr;
#endif
    }
};


template<typename PayloadT>
thread_local uint64_t ConcurrentHashMap<PayloadT>::rtm_use_lock = 0;
template<typename PayloadT>
thread_local uint64_t ConcurrentHashMap<PayloadT>::rtm_retry = 0;