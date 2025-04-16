set(SOURCES src/RoomBuilder.cpp)
add_library(mnt STATIC ${SOURCES})
target_link_libraries(mnt PUBLIC ecs math)
target_include_directories(mnt PUBLIC include)

