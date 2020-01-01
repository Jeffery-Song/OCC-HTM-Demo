project (occdb)

## CMake version
cmake_minimum_required(VERSION 2.8)

## Use C++11 features
add_definitions(-std=c++11)

## Set dependencies
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DOS_LINUX -mrtm -g -pthread")
set(BOOST_LIB "/home/sxn/local/boost_1_70_0-install/lib")
set(CMAKE_BUILD_TYPE Debug)

## Set include paths
include_directories(include)
include_directories(benchmark)

## Source code
file(GLOB SOURCES "include/*.hpp")

## Set options (cached)
## usage: cmake .. -DUSE_RTM=ON
option (USE_RTM "enable RTM" OFF)
if(USE_RTM)
  add_definitions(-DUSE_RTM)
endif(USE_RTM)

file(GLOB BENCH "benchmark/main.cc")
add_executable(bench ${BENCH})
target_link_libraries(bench)

file(GLOB TS  "tests/*.cc")
add_executable(coretest ${TS} )
target_link_libraries(coretest)