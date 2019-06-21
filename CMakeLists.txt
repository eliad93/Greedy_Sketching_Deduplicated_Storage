cmake_minimum_required(VERSION 3.14)
project(Greedy_Sketching_Deduplicated_Storage)

set(CMAKE_CXX_STANDARD 17)
add_executable(Greedy_Sketching_Deduplicated_Storage main.cpp ArgumentsParser.cpp ArgumentsParser.h System.cpp System.h Simulator.cpp Simulator.h)
target_link_libraries(${PROJECT_NAME}  stdc++fs)  # link to libstdc++fs