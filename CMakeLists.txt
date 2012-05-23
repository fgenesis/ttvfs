cmake_minimum_required(VERSION 2.6)
project(ttvfs)

option(TTVFS_BUILD_EXAMPLES "Build examples?" FALSE)
option(TTVFS_SUPPORT_ZIP "Build support for zip archives?" FALSE)
option(TTVFS_LARGEFILE_SUPPORT "Enable support for files > 4 GB? (experimental!)" FALSE)
option(TTVFS_IGNORE_CASE "Enable full case-insensitivity even on case-sensitive OSes like Linux and alike?" FALSE)
option(TTVFS_USE_HASHMAP "Use a faster hashmap instead of std::map for the tree? (experimental!)" FALSE)

# Be sure to copy this part to your root CMakeLists.txt if you prefer to use CMake for configuring
# instead of editing the config header directly!
# If you edit the header, this is not necessary.
if(TTVFS_LARGEFILE_SUPPORT)
    add_definitions("-DVFS_LARGEFILE_SUPPORT")
endif()
if(TTVFS_IGNORE_CASE)
    add_definitions("-DVFS_IGNORE_CASE")
endif()
if(TTVFS_USE_HASHMAP)
    add_definitions("-DVFS_USE_HASHMAP")
endif()
if(TTVFS_SUPPORT_ZIP)
    add_definitions("-DVFS_SUPPORT_ZIP")
endif()
# --snip--


# compiler specific things
if(MSVC)
    # MSVC builds require installed runtime library by default
    option(TTVFS_STATIC_LIB "Link as static library without runtime dependencies (Note: To get rid of this setting with MSVC, the cmake cache must be cleared)" FALSE)
    add_definitions("/GR-") # run-time type info (RTTI) not required
    
    if(TTVFS_STATIC_LIB)
        # this is ugly - hackfix compiler flags
        foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MD")
            if(${flag_var} MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MDd")
        endforeach()
        
        # hackfix linker flags - no idea why, but MSVC will produce linker errors otherwise
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:msvcrt.lib,msvcrtd.lib") # not sure if this is correct
        set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrt.lib,msvcrtd.lib")
    endif()
endif()

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
