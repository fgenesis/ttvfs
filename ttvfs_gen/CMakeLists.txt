
set(ttvfs_gen_SRC
    ttvfs_gen.cpp
)

include_directories(${TTVFS_INCLUDE_DIRS})

add_executable(ttvfs_gen ${ttvfs_gen_SRC})

target_link_libraries(ttvfs_gen ttvfs ttvfs_zip)

install(TARGETS ttvfs_gen DESTINATION bin)
