cmake_minimum_required(VERSION 3.22.1)
project(vulkanCore LANGUAGES C CXX)


# -------------------- SURFACE --------------------

add_library(vulkanCore SHARED
        native-lib.cpp
)

target_link_libraries(
        vulkanCore
)