cmake_minimum_required(VERSION 3.15)
project(project3)

set(CMAKE_CXX_STANDARD 11)

add_executable(simulator Simulator.cc
        Event.cc
        Link.cc
        Node.cc
        RoutingProtocolImpl.cc
        DetectNeighbor.cc DVManager.h common.h common.cc LSManager.h FloodingPath.cc)
add_executable(main main.cc)