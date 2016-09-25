
include_directories(${TTVFS_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(example1 example1.cpp)
add_executable(example2 example2.cpp)
add_executable(example3 example3.cpp)
add_executable(example6 example6.cpp)
add_executable(example8 example8.cpp)
add_executable(example10 example10.cpp)
add_executable(benchmark benchmark.cpp)
add_executable(dirlist dirlist.cpp)

target_link_libraries(example1 ttvfs)
target_link_libraries(example2 ttvfs)
target_link_libraries(example3 ttvfs)
target_link_libraries(example6 ttvfs)
target_link_libraries(example8 ttvfs)
target_link_libraries(example10 ttvfs)
target_link_libraries(benchmark ttvfs)
target_link_libraries(dirlist ttvfs)

if(TTVFS_SUPPORT_ZIP)
    add_executable(example4 example4.cpp)
    target_link_libraries(example4 ttvfs ttvfs_zip)

    add_executable(example5 example5.cpp)
    target_link_libraries(example5 ttvfs ttvfs_zip)

    add_executable(example9 example9.cpp)
    target_link_libraries(example9 ttvfs ttvfs_zip)

    if(TTVFS_BUILD_GENERATOR)
        add_custom_command(
            OUTPUT
                ${CMAKE_CURRENT_BINARY_DIR}/res.c
                ${CMAKE_CURRENT_BINARY_DIR}/res.h
            COMMAND ttvfs_gen ResourceData ResourceSize
                ${CMAKE_CURRENT_SOURCE_DIR}/res
                ${CMAKE_CURRENT_BINARY_DIR}/res.c
                ${CMAKE_CURRENT_BINARY_DIR}/res.h
            DEPENDS ttvfs_gen
            COMMENT "Generating resource file for example11"
        )
        add_executable(example11
            example11.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/res.c
            ${CMAKE_CURRENT_BINARY_DIR}/res.h
        )
        target_link_libraries(example11 ttvfs ttvfs_zip)
    endif()
endif()

if(TTVFS_BUILD_CFILEAPI)
    add_executable(example7 example7.cpp)
    target_link_libraries(example7 ttvfs_cfileapi)
endif()
