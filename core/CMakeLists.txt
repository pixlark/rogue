cmake_minimum_required(VERSION 3.29.3)
project(core)

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
# core
#

add_executable(core)
add_executable(core::core ALIAS core)

# I was not able to get this working...
# target_enable_cxx_modules(core)
# target_sources(core
#     PRIVATE
#         core/core.cc
# )
# target_cxx_module_sources(core
#     PRIVATE
#         core/core.ixx
# )

target_sources(core
    PRIVATE
        ascii.cc
        core.cc
        math.cc
        services.cc
        world.cc
        generation/room-generation.cc
)
target_compile_features(core
    PUBLIC
        cxx_std_23
)
target_enable_warnings(core)
# target_compile_options(core PRIVATE /analyze /analyze:plugin EspXEngine.dll)
target_link_libraries(core
    PRIVATE
        backend::backend
        Microsoft.GSL::GSL
)
target_install_executable(core)
