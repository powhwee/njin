set(SOURCES src/EventHandler.cpp)

add_library(input STATIC ${SOURCES})
target_include_directories(input PUBLIC include)
target_link_libraries(input PRIVATE SDL3::SDL3)

