
include_directories(${TTVFS_INCLUDE_DIRS})

add_executable(example1 example1.cpp)
add_executable(example2 example2.cpp)
add_executable(example3 example3.cpp)
add_executable(example6 example6.cpp)
add_executable(benchmark benchmark.cpp)
add_executable(dirlist dirlist.cpp)

target_link_libraries(example1 ttvfs)
target_link_libraries(example2 ttvfs)
target_link_libraries(example3 ttvfs)
target_link_libraries(example6 ttvfs)
target_link_libraries(benchmark ttvfs)
target_link_libraries(dirlist ttvfs)

if(TTVFS_SUPPORT_ZIP)
    add_executable(example4 example4.cpp)
    target_link_libraries(example4 ttvfs ttvfs_zip)

    add_executable(example5 example5.cpp)
    target_link_libraries(example5 ttvfs ttvfs_zip)
endif()
