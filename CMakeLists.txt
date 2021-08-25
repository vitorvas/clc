cmake_minimum_required (VERSION 3.0)
project(CrossCorr)

find_package(OpenCL REQUIRED)

set(CL_TARGET_OPENCL_VERSION "1.1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-deprecated-declarations")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable(ccr clcrosscorr.c)
target_link_libraries(ccr m ${OpenCL_LIBRARY})


