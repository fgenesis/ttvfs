cmake_minimum_required(VERSION 2.6)
project(ttvfs)

option(TTVFS_BUILD_EXAMPLES "Build examples?" FALSE)
option(TTVFS_SUPPORT_ZIP "Build support for zip archives?" FALSE)
option(TTVFS_LARGEFILE_SUPPORT "Enable support for files > 4 GB? (experimental!)" TRUE)
option(TTVFS_IGNORE_CASE "Enable full case-insensitivity even on case-sensitive OSes like Linux and alike?" TRUE)

# Be sure to copy this part to your root CMakeLists.txt if you prefer to use CMake for configuring
# instead of editing the config header directly!
# If you edit the header, this is not necessary.
if(TTVFS_LARGEFILE_SUPPORT)
    add_definitions("-DVFS_LARGEFILE_SUPPORT")
endif()
if(TTVFS_IGNORE_CASE)
    add_definitions("-DVFS_IGNORE_CASE")
endif()
if(TTVFS_SUPPORT_ZIP)
    add_definitions("-DVFS_SUPPORT_ZIP")
endif()
# --snip--



# non-msvc needs build type - if no build type was provided, set a default one
if(NOT MSVC)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
    endif()
endif()

set(code_dirs "${CMAKE_CURRENT_SOURCE_DIR}/ttvfs")

if(TTVFS_SUPPORT_ZIP)
    list(APPEND code_dirs "${CMAKE_CURRENT_SOURCE_DIR}/ttvfs_zip")
endif()

message(STATUS "Files dir: ${code_dirs}")

set(TTVFS_INCLUDE_DIRS "${code_dirs}" CACHE STRING "ttvfs include directory - for external includers" FORCE)
set(TTVFS_SRC_DIR "${code_dirs}" CACHE STRING "ttvfs source directory - for external includers" FORCE)


add_subdirectory(ttvfs)

if(TTVFS_SUPPORT_ZIP)
    add_subdirectory(ttvfs_zip)
endif()

if(TTVFS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
