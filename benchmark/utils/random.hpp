#pragma once

#include <cstdint>
#include <random>
#include "assertion.hpp"

class RandUInt {
  public:
    virtual uint32_t next() {
        return 0;
    }
};

class RandUIntGauss : public RandUInt {
  private:
    uint32_t min, max;
    std::normal_distribution<> dist;
    std::random_device r;
    std::default_random_engine engine;
  public:
    RandUIntGauss() : RandUIntGauss(0, (uint32_t)-1) {}
    RandUIntGauss(uint32_t min_, uint32_t max_) : 
        min(min_), max(max_), r(), engine(r()), dist(0, 1) {
        ASSERT_TRUE(min_ < max_);
    }
    uint32_t next() override {
        double gauss = dist(engine);
        gauss = (gauss - 2.0) / 4.0;
        return std::round(gauss * (max - min)) + min;
    }
};

class RandUIntUniform : public RandUInt {
  private:
    uint32_t min, max;
    std::uniform_int_distribution<> dist;
    std::random_device r;
    std::default_random_engine engine;
  public:
    RandUIntUniform() : RandUIntUniform(0, (uint32_t)-1) {}
    RandUIntUniform(uint32_t min_, uint32_t max_) : 
        min(min_), max(max_), r(), engine(r()), dist(min_, max_) {
        ASSERT_TRUE(min_ <= max_);
    }
    uint32_t next() override {
        return dist(engine);
    }
};

class RandUIntHot : public RandUInt {
  private:
    uint32_t hot_prob;
    RandUIntUniform cold, hot, dohot;
  public:
    RandUIntHot(uint32_t min_, uint32_t max_, 
                uint32_t max_hot_, uint32_t hot_prob_) : 
            hot_prob(hot_prob_), 
            hot(min_, min_ + max_hot_), 
            cold(min_ + max_hot_ + 1, max_), 
            dohot(0, 99) {

    }
    uint32_t next() override {
        return dohot.next() < hot_prob ? hot.next() : cold.next();
    }
};