cmake_minimum_required(VERSION 3.9)
project(tcp_server_example VERSION 1.0.0 DESCRIPTION "tcp server lib. Preforced TCP server")

########################################################################
# executables
########################################################################
add_executable(
    tcp_server_example
    "server/main.c"
)

target_include_directories(tcp_server_example PUBLIC include/)
target_link_libraries(tcp_server_example LINK_PRIVATE tcp_server)
