#pragma once
#include <chrono>

using timepoint_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

template<typename TUnit>
uint64_t dura_cast(timepoint_t start, timepoint_t stop) {
    return std::chrono::duration_cast<TUnit>(stop - start).count();
}
uint64_t seconds(timepoint_t start, timepoint_t stop) {
    return dura_cast<std::chrono::seconds>(start, stop);
}
uint64_t microseconds(timepoint_t start, timepoint_t stop) {
    return dura_cast<std::chrono::microseconds>(start, stop);
}
uint64_t nanoseconds(timepoint_t start, timepoint_t stop) {
    return dura_cast<std::chrono::nanoseconds>(start, stop);
}

timepoint_t now() {
    return std::chrono::high_resolution_clock::now();
}