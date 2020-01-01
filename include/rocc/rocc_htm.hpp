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

#define MAXNEST 0
#define NESTSTEP 0
#define MAXZERO 0

//This number is just for performance debugging
//#define MAXCAPACITY 1024
#define MAXCAPACITY 1

#define MAXCONFLICT 100

#define MAXWRITE 64
#define MAXREAD 128

#define SIMPLERETY 0

class RTMScope {

    int retry;
    int conflict;
    int capacity;
    int nested;
    int zero;
    uint64_t befcommit;
    uint64_t aftcommit;
    SpinLock* slock;

  public:

    static SpinLock fblock;

    inline RTMScope(SpinLock* sl = NULL) {
        //  inline RTMScope(TXProfile* prof, int read = 1, int write = 1, SpinLock* sl = NULL) {

        retry = 0;
        conflict = 0;
        capacity = 0;
        zero = 0;
        nested = 0;

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

                retry++;
                //if (prof!= NULL) prof->recordAbortStatus(stat);
                if((stat & _XABORT_NESTED) != 0)
                    nested++;
                else if(stat == 0)
                    zero++;
                else if((stat & _XABORT_CONFLICT) != 0) {
                    conflict++;
                }
                else if((stat & _XABORT_CAPACITY) != 0)
                    capacity++;

                if((stat & _XABORT_EXPLICIT) && _XABORT_CODE(stat) == 0xff) {
                    while(slock->IsLocked())
                        _mm_pause();
                }
                if((stat & _XABORT_EXPLICIT) && _XABORT_CODE(stat) == 0x73) {
                    //retry maybe not helpful
                    break;
                }

#if SIMPLERETY
                if(retry > 100)
                    break;
#else

                int step = 1;

                if (nested > MAXNEST)
                    break;
                if(zero > MAXZERO/step) {
                    break;
                }

                if(capacity > MAXCAPACITY / step) {
                    break;
                }
                if (conflict > MAXCONFLICT/step) {
                    break;
                }
#endif
            }
        }
        slock->Lock();
    }

    void Abort() {
        _xabort(0x1);
    }

    inline  ~RTMScope() {
        if(slock->IsLocked())
            slock->Unlock();
        else
            _xend ();
    }

  private:
    RTMScope(const RTMScope&);
    void operator=(const RTMScope&);
};

SpinLock RTMScope::fblock;