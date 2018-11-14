cmake_minimum_required(VERSION 3.0)
project(chbl LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

include(CheckIncludeFile)
check_include_file(ctype.h  HAVE_CTYPE_H)
check_include_file(math.h   HAVE_MATH_H)
check_include_file(stdio.h  HAVE_STDIO_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(unistd.h HAVE_UNISTD_H)

add_executable(chbl chbl.c)
target_link_libraries(chbl m)

install(TARGETS chbl RUNTIME DESTINATION bin)
install(FILES chbl.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)
