cmake_minimum_required(VERSION 3.29.3)
project(core)

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
        common/logging.cc
        common/math.cc
        common/services.cc
        core/ascii.cc
        core/core.cc
        game/world.cc
        generation/room-generation.cc
)
target_enable_cxx23(core)
target_enable_warnings(core)
target_include_directories(core
    PRIVATE
        .
)
# target_compile_options(core PRIVATE /analyze /analyze:plugin EspXEngine.dll)
target_link_libraries(core
    PRIVATE
        backend::backend
        Microsoft.GSL::GSL
)
target_install_executable(core)

#
# tests
#

add_executable(unit-tests)
add_executable(core::unit-tests ALIAS unit-tests)
target_sources(unit-tests
    PRIVATE
        tests/unit-tests.cc
)
target_enable_cxx23(unit-tests)
target_enable_warnings(unit-tests)
target_include_directories(unit-tests
    PRIVATE
        .
)
target_link_libraries(unit-tests
    PRIVATE
        Microsoft.GSL::GSL
        test-platform::test-platform
)
target_install_executable(unit-tests)

#
# cgreen
#

# include(FetchContent)

# FetchContent_Declare(cgreen
#     GIT_REPOSITORY "git@github.com:cgreen-devs/cgreen.git"
#     GIT_TAG "1.6.3"
#     GIT_SHALLOW true
#     EXCLUDE_FROM_ALL
# )

# FetchContent_MakeAvailable(cgreen)

# target_include_directories(cgreen_shared
#     INTERFACE
#         ${CGREEN_PUBLIC_INCLUDE_DIRS}
# )

# target_enable_cxx23(cgreen_shared OLD_LINK_LIBRARIES_COMPAT)

#
# tests
#

# add_executable(tests)
# add_executable(tests::tests ALIAS tests)

# target_sources(tests
#     PRIVATE
#         tests.cc
# )
# target_enable_cxx23(tests)
# target_enable_warnings(tests)
# target_link_libraries(tests
#     PRIVATE
#         Microsoft.GSL::GSL
#         cgreen_shared
# )
# if(UNIX)
#     set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
# endif()
# target_install_executable(tests)
