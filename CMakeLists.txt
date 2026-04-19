cmake_minimum_required(VERSION 4.1)
project(Semestral)

set(CMAKE_C_STANDARD 26)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 26)

add_compile_options(
        -fsanitize=address
        -fsanitize=undefined
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer
        -g
        -fstack-protector-strong
        -Wno-unused-result
        -Wall
        -Wextra
        -Werror=uninitialized
        -Werror=vla
        -Werror=return-type
        -Wshadow
        -Wnull-dereference
        -Wformat
        -Wformat-security
        -Wstrict-overflow
        -Wstrict-prototypes
        -Wcast-align
        -Wredundant-decls
        -Wpedantic
        -Werror
        -Wno-shadow
)

add_link_options(
        -fsanitize=address
        -fsanitize=undefined
)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(Semestral ${SOURCES})

target_compile_options(Semestral PRIVATE -Wall -Wextra -Wpedantic)
