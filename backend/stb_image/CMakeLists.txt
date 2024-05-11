cmake_minimum_required(VERSION 3.29.3)
project(stb_image)

add_library(stb_image INTERFACE)
add_library(stb_image::stb_image ALIAS stb_image)

target_include_directories(stb_image
    INTERFACE
        .
)
