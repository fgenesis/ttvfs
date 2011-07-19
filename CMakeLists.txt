cmake_minimum_required(VERSION 2.6)
project(ttvfs)

option(TTVFS_BUILD_EXAMPLES "Build examples?" FALSE)

# non-msvc needs build type - if no build type was provided, set a default one
if(NOT MSVC)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
    endif()
endif()

add_subdirectory(ttvfs)

if(TTVFS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
