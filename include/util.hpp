#pragma once

#include "assertion.hpp"
#include "type.hpp"

#include <cstdint>

using KeyType = uint32_t;

#ifndef PayloadType
#define PayloadType uint32_t
#endif

enum RC {
    Ok = 0,
    Abort,
    Not_Exist,
    Already_Exist,
};