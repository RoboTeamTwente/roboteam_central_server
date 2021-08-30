cmake_minimum_required(VERSION 3.16)
project(roboteam_central_server VERSION 0.1.0)

add_executable(roboteam_central_server 
        main.cpp
        src/server.cpp
        )

target_link_libraries(roboteam_central_server
        PRIVATE stx
        PRIVATE roboteam_utils
        PRIVATE ixwebsocket
        PRIVATE roboteam_proto
        PRIVATE roboteam_utils
        )