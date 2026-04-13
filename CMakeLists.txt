cmake_minimum_required(VERSION 3.9)
project(tcp_server VERSION 1.0.0 DESCRIPTION "tcp server lib. Preforced TCP server")

########################################################################
# tcp_server lib
########################################################################
file(GLOB_RECURSE SOURCES "src/*.c")
add_library(tcp_server SHARED ${SOURCES})

set_target_properties(tcp_server PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    PUBLIC_HEADER include/tcp_server.h)

target_include_directories(tcp_server PRIVATE src/)
target_include_directories(tcp_server PUBLIC include/)

########################################################################
# executables
########################################################################

########################################################################
# examples
########################################################################
add_subdirectory(examples)
