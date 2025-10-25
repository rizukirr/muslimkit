cmake_minimum_required(VERSION 4.0)
project(muslimkit LANGUAGES C)

set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -fsanitize=address -g -O1")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(OpenSSL REQUIRED)
if (NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found! "
    "Please install libssl-dev (Debian/Ubuntu) or openssl (Arch/Fedora)")
endif()

file(GLOB_RECURSE SRC_FILES "src/*.c")
add_executable(muslimkit main.c ${SRC_FILES})
target_include_directories(muslimkit PUBLIC include)

target_link_libraries(muslimkit
                        OpenSSL::SSL
                        OpenSSL::Crypto)
