cmake_minimum_required(VERSION 3.0)
project(chbl LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

add_executable(chbl chbl.c)
target_link_libraries(chbl m)

install(TARGETS chbl RUNTIME DESTINATION bin)
