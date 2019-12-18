#pragma once

#include <cstdint>
#include <string>

#include "transaction.hpp"

struct Account {
    uint32_t id;
    std::string name;
    uint32_t saving;
    uint32_t checks;
};

using SBTxn = Transaction<Account>;
using SBDb = ConcurrentHashMap<Account>;
