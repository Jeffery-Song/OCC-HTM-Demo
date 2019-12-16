#pragma once
#include <cstdint>
#include <string>
// small bank

struct Account {
    uint32_t id;
    std::string name;
    uint32_t saving;
    uint32_t checks;
};