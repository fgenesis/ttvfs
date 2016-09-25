cmake_minimum_required(VERSION 2.6)
project(ttvfs)

option(TTVFS_BUILD_EXAMPLES "Build examples?" FALSE)
option(TTVFS_BUILD_GENERATOR "Build the embedded zip archive generator tool." TRUE)
option(TTVFS_SUPPORT_ZIP "Build support for zip archives?" TRUE)
option(TTVFS_LARGEFILE_SUPPORT "Enable support for files > 4 GB? (experimental!)" TRUE)
option(TTVFS_IGNORE_CASE "Enable full case-insensitivity even on case-sensitive OSes like Linux and alike?" TRUE)
option(TTVFS_BUILD_CFILEAPI "Build C-style API wrapper" TRUE)
option(TTVFS_BUILD_TESTS "Build tests" FALSE)

if(TTVFS_BUILD_GENERATOR AND NOT TTVFS_SUPPORT_ZIP)
    message(FATAL_ERROR "You must enable TTVFS_SUPPORT_ZIP to use TTVFS_BUILD_GENERATOR")
endif()

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
ENDIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

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
if(TTVFS_BUILD_CFILEAPI)
    list(APPEND code_dirs "${CMAKE_CURRENT_SOURCE_DIR}/ttvfs_cfileapi")
endif()

message(STATUS "Files dir: ${code_dirs}")

set(TTVFS_INCLUDE_DIRS "${code_dirs}" CACHE STRING "ttvfs include directory - for external includers" FORCE)
set(TTVFS_SRC_DIR "${code_dirs}" CACHE STRING "ttvfs source directory - for external includers" FORCE)


add_subdirectory(ttvfs)

if(TTVFS_SUPPORT_ZIP)
    add_subdirectory(ttvfs_zip)
endif()

if(TTVFS_BUILD_CFILEAPI)
    add_subdirectory(ttvfs_cfileapi)
endif()

if(TTVFS_BUILD_GENERATOR AND TTVFS_SUPPORT_ZIP)
    add_subdirectory(ttvfs_gen)
endif()

if(TTVFS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(TTVFS_BUILD_TESTS)
    add_subdirectory(test)
endif()

