#pragma once

#include "assertion.hpp"
#include "type.hpp"

#include <cstdint>

using KeyType = uint32_t;

enum RC {
    Ok = 0,
    Abort,
    Not_Exist,
    Already_Exist,
};