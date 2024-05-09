cmake_minimum_required(VERSION 3.29.3)
project(backend)

# add_subdirectory(SDL)

add_library(backend SHARED EXCLUDE_FROM_ALL)
add_library(backend::backend ALIAS backend)

set_target_properties(backend
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS true
)
target_sources(backend
    PRIVATE
        backend.c
)
target_include_directories(backend
    INTERFACE
        .
)
