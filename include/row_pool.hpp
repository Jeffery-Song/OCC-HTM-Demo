#pragma once
#include "row.hpp"
#include <boost/pool/pool.hpp>


template<typename PayloadT>
class RowPool {
  private:
    boost::pool<> _pool;
  public:
    RowPool() : _pool(sizeof(Row<PayloadT>)) {}
    void* malloc() {
        void * rst = _pool.malloc();
        rst = new(rst) Row<PayloadT>;
        return rst;
    }
    void free(void* ptr) {
        ((Row<PayloadT>*)ptr)->~Row<PayloadT>();
        _pool.free(ptr);
    }
};