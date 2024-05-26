cmake_minimum_required(VERSION 3.29.3)
project(test-platform)

#
# GSL
#

include(FetchContent)

FetchContent_Declare(GSL
    GIT_REPOSITORY "git@github.com:microsoft/GSL.git"
    GIT_TAG "v4.0.0"
    GIT_SHALLOW true
)

FetchContent_MakeAvailable(GSL)

#
# test-platform
#

add_library(test-platform SHARED)
add_library(test-platform::test-platform ALIAS test-platform)
set_target_properties(test-platform
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS true
)

target_sources(test-platform
    PRIVATE
        test-platform.cc
)
target_enable_cxx23(test-platform)
target_enable_warnings(test-platform)
target_include_directories(test-platform
    PUBLIC
        .
)
target_link_libraries(test-platform
    PRIVATE
        backend::backend
        Microsoft.GSL::GSL
)
