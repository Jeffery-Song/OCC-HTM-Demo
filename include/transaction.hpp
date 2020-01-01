#pragma once
#include "row.hpp"
#include "concurrent_hash_map.hpp"
#include "util.hpp"
#include <map>
#include <set>


#ifdef USE_RTM
#include "rocc/rocc_htm.hpp"
#else
#include "rw_spin_lock.hpp"
#endif


template<typename PayloadT>
class Transaction {
  private:
    struct buff_item {
        Row<PayloadT>* ptr = nullptr;
        uint64_t version = 0;
        PayloadT data;
    };
    std::map<KeyType, buff_item> buffer;
    std::set<KeyType> wset, iset;
    // std::map<KeyType, buff_item> wset;

    ConcurrentHashMap<PayloadT> * table;

    SpinLock _txn_lock;

    buff_item* try_local(KeyType key, bool & buffed, bool & exist) {
        buffed = false;
        exist = false;
        auto iter = buffer.find(key);
        if (iter == buffer.end()) {
            return nullptr;
        }

        buffed = true;
        if (iter->second.version == 0 && iset.find(key) == iset.end()) {
            return nullptr;
        }
        // fprintf(stderr, "hit %u in buffer, val is %u, vn is %lu\n", key, iter->second.data, iter->second.version);
        exist = true;
        return &iter->second;
    }

    buff_item & buff_from_db(KeyType key) {
        Row<PayloadT>* row = (Row<PayloadT>*)table->Read_or_Insert(key);
        buff_item buff_row;
        buff_row.ptr = row;
#ifdef USE_RTM
        {
            RTMScope rtm(&_txn_lock); {
                buff_row.data = row->payload;
                buff_row.version = row->get_version();
            }
        }
#else
        row->lock();
        buff_row.data = row->payload;
        buff_row.version = row->get_version();
        row->unlock();
#endif
        buffer[key] = buff_row;
        // fprintf(stderr, "extract %u in db, val is %u, vn is %lu\n", key, buffer[key].data, buffer[key].version);
        return buffer[key];
    } 

    void lock_wset() {
        // lock wset
        for (auto key : wset) {
            buffer.at(key).ptr->lock();
        }
    }
    void unlock_wset() {
        // unlock wset
        for (auto key : wset) {
            buffer.at(key).ptr->unlock();
        }
    }
  public:
    Transaction(ConcurrentHashMap<PayloadT> * table) : table(table) {}
    PayloadT Read(KeyType key, RC* rc = nullptr) {
        bool buffed, exist;
        buff_item * buff_row_ptr = try_local(key, buffed, exist);
        if (exist) {
            if (rc) *rc = RC::Ok;
            return buff_row_ptr->data;
        }
        if (buffed) {
            if (rc == nullptr) {ASSERT_MSG(false, "Key not exist");}
            *rc = RC::Not_Exist;
            return PayloadT();
        }

        buff_item & buff_row = buff_from_db(key);

        if (buff_row.version == 0) {
            if (rc == nullptr) {ASSERT_MSG(false, "Key not exist");}
            *rc = RC::Not_Exist;
            return PayloadT();
        }

        if (rc) *rc = RC::Ok;
        // fprintf(stderr, "read %u, val is %u, vn is %lu\n", key, buff_row.data, buff_row.version);
        return buff_row.data;
    }
    void Update(KeyType key, PayloadT p, RC* rc = nullptr) {
        bool buffed, exist;
        buff_item * buff_row_ptr = try_local(key, buffed, exist);
        if (exist) {
            if (rc) *rc = RC::Ok;
            buff_row_ptr->data = p;
            wset.insert(key);
            return;
        }
        if (buffed) {
            if (rc == nullptr) {ASSERT_MSG(false, "Key not exist");}
            *rc = RC::Not_Exist;
            return;
        }

        buff_item & buff_row = buff_from_db(key);

        if (buff_row.version == 0) {
            if (rc == nullptr) {ASSERT_MSG(false, "Key not exist");}
            *rc = RC::Not_Exist;
            return;
        }
        buff_row.data = p;
        wset.insert(key);
        if (rc) *rc = RC::Ok;
    }
    void Insert(KeyType key, PayloadT p, RC* rc = nullptr) {
        bool buffed, exist;
        buff_item * buff_row_ptr = try_local(key, buffed, exist);
        if (exist) {
            if (rc == nullptr) {ASSERT_MSG(false, "Key already exist");}
            *rc = RC::Already_Exist;
            return;
        }
        if (buffed) {
            buff_row_ptr->data = p;
            iset.insert(key);
            wset.insert(key);
            if (rc) *rc = RC::Ok;
            return;
        }

        buff_item & buff_row = buff_from_db(key);

        if (buff_row.version > 0) {
            if (rc == nullptr) {ASSERT_MSG(false, "Key already exist");}
            *rc = RC::Already_Exist;
            return;
        }
        buff_row.data = p;
        iset.insert(key);
        wset.insert(key);
        if (rc) *rc = RC::Ok;
    }
    bool Validate() {
#ifdef USE_RTM
        for (auto & iter : buffer) {
            if (iter.second.ptr->get_version() != iter.second.version) {
                // validation fail
                return false;
            }
        }
        return true;
#else
        bool success = true;
        // lock_wset();

        for (auto & iter : buffer) {
            bool should_unlock = false;
            if (wset.find(iter.first) == wset.end()) {
                should_unlock = true;
                iter.second.ptr->lock();
            }
            if (iter.second.ptr->get_version() != iter.second.version) {
                // validation fail
                success = false;
                if (should_unlock) iter.second.ptr->unlock();
                break;
            }
            if (should_unlock) iter.second.ptr->unlock();
        }
        return success;
#endif
    }
    void WriteBack() {
        for (auto key : wset) {
            // fprintf(stderr, "making key=%u from val=%u,vn=%lu to val=%u,vn=%lu\n", 
            //         key, 
            //         buffer.at(key).ptr->payload, buffer.at(key).ptr->get_version(), 
            //         buffer.at(key).data, buffer.at(key).ptr->get_version()+1);
            buffer.at(key).ptr->payload = buffer.at(key).data;
            buffer.at(key).ptr->inc_version();
        }
        // unlock_wset();
    }
    bool Commit() {
        bool ret = false;;
#ifdef USE_RTM
        {
            RTMScope rtm(&_txn_lock);{
                if (Validate()) {
                    ret = true;
                    WriteBack();
                }
            }
        }
#else
        lock_wset();
        if (Validate()) {
            ret = true;
            WriteBack();
        }
        unlock_wset();
#endif
        clear();
        return ret;
    }
    void clear() {
        buffer.clear();
        wset.clear();
        iset.clear();
    }
};