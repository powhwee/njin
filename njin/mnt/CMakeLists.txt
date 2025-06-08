set(SOURCES
    src/RoomBuilder.cpp
    src/Level.cpp
    src/TownLevel.cpp
)
add_library(mnt STATIC ${SOURCES})
target_link_libraries(mnt PUBLIC ecs math)
target_include_directories(mnt PUBLIC include)

