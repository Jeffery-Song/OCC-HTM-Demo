project (occdb)

## CMake version
cmake_minimum_required(VERSION 2.8)

## Use C++11 features
add_definitions(-std=c++14)

## Set dependencies
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DOS_LINUX -mrtm -g -pthread")
set(BOOST_LIB "/home/sxn/local/boost_1_70_0-install/lib")
set(CMAKE_BUILD_TYPE Debug)

## Set include paths
include_directories(include)
include_directories(benchmark)

## Source code
file(GLOB SOURCES "include/*.hpp")

## Set options (cached)
## usage: cmake .. -DCMAKE_USE_RTM_TXN=ON
option (CMAKE_USE_RTM_TXN "enable RTM in transaction operation" ON)
if(CMAKE_USE_RTM_TXN)
  add_definitions(-DUSE_RTM_TXN)
endif(CMAKE_USE_RTM_TXN)

option (CMAKE_USE_RTM_STORE "enable RTM in storage operation" ON)
if(CMAKE_USE_RTM_STORE)
  add_definitions(-DUSE_RTM_STORE)
endif(CMAKE_USE_RTM_STORE)

file(GLOB BENCH "benchmark/main.cc")
add_executable(bench ${BENCH})
target_link_libraries(bench)

file(GLOB TS  "tests/*.cc")
add_executable(coretest ${TS} )
target_link_libraries(coretest)