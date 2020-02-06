cmake_minimum_required(VERSION 3.13)
project(project2)

set(CMAKE_CXX_STANDARD 14)

#add_executable(project2 server_num.c name_addr.c client_num.c)
add_executable(project2 client_num.c server_num.c web_server.c web_server.h)
#add_executable(project2 server_num.c)
#add_executable(project2 name_addr.c)
