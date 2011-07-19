
# copied this part from the ttvfs lib CMakeLists.txt so that the examples
# will use the same compile config as ttvfs even if configured with CMake only.
if(TTVFS_LARGEFILE_SUPPORT)
    add_definitions("-DVFS_LARGEFILE_SUPPORT")
endif()
if(TTVFS_IGNORE_CASE)
    add_definitions("-DVFS_IGNORE_CASE")
endif()


# compiler specific things - shortened version
if(MSVC)
    # MSVC builds require installed runtime library by default
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

# if you strip the bells and whistles, this is the only important part.

include_directories(${TTVFS_INCLUDE_DIRS})

add_executable(example1 example1.cpp)
add_executable(example2 example2.cpp)
add_executable(example3 example3.cpp)

target_link_libraries(example1 ttvfs)
target_link_libraries(example2 ttvfs)
target_link_libraries(example3 ttvfs)

