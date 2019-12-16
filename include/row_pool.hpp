#pragma once
#include "row.hpp"
#include <boost/pool/pool.hpp>

// template<typename PayloadT>

class RowPool {
  private:
    using PayloadT = PayloadType;
    boost::pool<> _pool;
  public:
    RowPool() : _pool(sizeof(Row)) {}
    void* malloc() {
        void * rst = _pool.malloc();
        rst = new(rst) Row;
        return rst;
    }
    void free(void* ptr) {
        ((Row*)ptr)->~Row();
        _pool.free(ptr);
    }
};