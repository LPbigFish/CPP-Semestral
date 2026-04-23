cmake_minimum_required(VERSION 4.1)
project(Semestral)

set(CMAKE_C_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)

include(CTest)

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

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
  -Wno-shadow)

add_link_options(-fsanitize=address -fsanitize=undefined)

file(GLOB_RECURSE SOURCES src/*.cpp)
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(SemestralLib STATIC ${SOURCES})
target_link_libraries(SemestralLib PRIVATE OpenSSL::Crypto)

add_executable(Semestral src/main.cpp)
target_link_libraries(Semestral PRIVATE SemestralLib)

file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)

add_executable(SemestralTests ${TEST_SOURCES})

target_link_libraries(SemestralTests PRIVATE SemestralLib GTest::gtest
                                             GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(SemestralTests)
