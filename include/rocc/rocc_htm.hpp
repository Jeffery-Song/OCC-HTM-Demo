/*
 *  The code is part of our project called DrTM, which leverages HTM and RDMA for speedy distributed
 *  in-memory transactions.
 *
 *
 * Copyright (C) 2015 Institute of Parallel and Distributed Systems (IPADS), Shanghai Jiao Tong University
 *     All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  For more about this software, visit:  http://ipads.se.sjtu.edu.cn/drtm.html
 *
 */


/*
 *  RTM UTIL         - Hao Qian
 *  RTM UTIL         - XingDa
 */

#pragma once
#include <immintrin.h>
#include <sys/time.h>
#include <assert.h>
#include <cstdint>
#include "rocc/spinlock.hpp"


#define MAXRETRY 100


#define SIMPLERETY 1

class RTMScope {

    int retry;
    SpinLock* slock;
    uint64_t & retry_ret;
    uint64_t & use_lock;
    bool use_lock_;

  public:

    static SpinLock fblock;

    inline RTMScope(uint64_t & retry_ret, uint64_t & use_lock, SpinLock* sl = NULL)
         : retry_ret(retry_ret), use_lock(use_lock) {
        //  inline RTMScope(TXProfile* prof, int read = 1, int write = 1, SpinLock* sl = NULL) {

        retry = 0;
        use_lock_ = false;

        if(sl == NULL) {
            //If the user doesn't provide a lock, we give him a default locking
            slock = &fblock;
        } else {
            slock = sl;
        }

        while(true) {
            unsigned stat;
            stat = _xbegin();
            if(stat == _XBEGIN_STARTED) {

                //Put the global lock into read set
                if(slock->IsLocked())
                    _xabort(0xff);

                return;

            } else {

#if SIMPLERETY
                retry++;
#endif
                if((stat & _XABORT_EXPLICIT) && _XABORT_CODE(stat) == 0xff) {
                    while(slock->IsLocked())
                        _mm_pause();
                }
                if((stat & _XABORT_EXPLICIT) && _XABORT_CODE(stat) == 0x73) {
                    //retry maybe not helpful
                    break;
                }

#if SIMPLERETY
                if(retry > MAXRETRY)
                    break;
#endif
            }
        }
        slock->Lock();
        use_lock_ = true;
    }

    void Abort() {
        _xabort(0x1);
    }

    inline  ~RTMScope() {
        if(use_lock_) {
            slock->Unlock();
            use_lock += 1;
        } else {
            _xend ();
        }
        retry_ret += retry;
    }

  private:
    RTMScope(const RTMScope&);
    void operator=(const RTMScope&);
};

SpinLock RTMScope::fblock;