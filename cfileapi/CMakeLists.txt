
set(cfileapi_SRC
    ttvfs_stdio.cpp
    ttvfs_stdio.h
)

include_directories(${TTVFS_INCLUDE_DIRS})

add_library(cfileapi ${cfileapi_SRC})

target_link_libraries(cfileapi ttvfs)

