cmake_minimum_required(VERSION 3.29.3)
project(demos)

add_executable(basic-backend-integration)
target_sources(basic-backend-integration
    PRIVATE
        "./basic-backend-integration/main.c"
)
target_enable_warnings(basic-backend-integration)
target_link_libraries(basic-backend-integration
    PRIVATE
        backend::backend
)
target_install_executable(basic-backend-integration)

add_executable(pathfinding)
target_sources(pathfinding
    PRIVATE
        "./pathfinding/main.cc"
        "../core/common/math.cc"
)
target_include_directories(pathfinding
    PRIVATE
        "../core"
)
target_enable_warnings(pathfinding)
target_enable_cxx23(pathfinding)
target_link_libraries(pathfinding
    PRIVATE
        backend::backend
        Microsoft.GSL::GSL
)
target_install_executable(pathfinding)
