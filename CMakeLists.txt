cmake_minimum_required(VERSION 3.0)
project(chbl)

include(CheckIncludeFile)
check_include_file(dirent.h    HAVE_DIRENT_H)
check_include_file(sys/stat.h  HAVE_SYS_STAT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(unistd.h    HAVE_UNISTD_H)

add_executable(chbl chbl.cpp)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
install(TARGETS chbl RUNTIME DESTINATION bin)
