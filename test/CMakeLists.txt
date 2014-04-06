
include_directories(${TTVFS_INCLUDE_DIRS})

add_executable(test1 test1.cpp)

target_link_libraries(test1 ttvfs)

