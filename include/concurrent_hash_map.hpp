#pragma once
#include "row.hpp"
#include "util.hpp"
#include "rw_spin_lock.hpp"
#include "row.hpp"
#include "row_pool.hpp"
#include <vector>
#include <unordered_map>
#include <boost/pool/pool.hpp>

class ConcurrentHashMap {
  private:
    mutable RWSpinLock _lock;
    std::unordered_map<KeyType, void*> _db;
    RowPool *row_pool;
  public:
    ConcurrentHashMap(RowPool * row_pool) : _lock(false), row_pool(row_pool) {
    }
    void* Read_or_Insert(const KeyType key) {
        _lock.rlock();
        auto iter = _db.find(key);
        if (iter != _db.end()) {
            void* r = _db.at(key);
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
        void* ptr = row_pool->malloc();
        _db[key] = ptr;
        _lock.unlock();
        return ptr;

    }
    void* Read(const KeyType key) const {
        _lock.rlock();
        const auto & iter = _db.find(key);
        if (iter == _db.end()) {
            _lock.unlock();
            return nullptr;
        }
        void* r = _db.at(key);
        _lock.unlock();
        return r;
    }
};

