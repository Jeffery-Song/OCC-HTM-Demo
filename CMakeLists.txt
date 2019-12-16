project (occdb)

## CMake version
cmake_minimum_required(VERSION 2.8)

## Use C++11 features
add_definitions(-std=c++11)

## Set dependencies
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -pthread")
set(BOOST_LIB "/home/sxn/local/boost_1_70_0-install/lib")
set(CMAKE_BUILD_TYPE Debug)

## Set include paths
include_directories(include)

## Source code
file(GLOB SOURCES "include/*.hpp")


## Set options (cached)
## usage: cmake .. -DUSE_RDMA=OFF -DUSE_HADOOP=ON

# #### RDMA
# option (USE_RDMA "enable RDMA support" OFF)
# if(USE_RDMA)
#   add_definitions(-DHAS_RDMA)
# endif(USE_RDMA)

## Build Wukong
# add_executable(server ${SOURCES} "src/server_main.cpp")
# target_link_libraries(server)

file(GLOB TS  "tests/*.cc")
add_executable(coretest ${TS} )
target_link_libraries(coretest)



