cmake_minimum_required(VERSION 3.29.3)
project(samples)

add_executable(backend-test)
target_sources(backend-test
    PRIVATE
        "./backend-test/backend-test.c"
)
target_enable_warnings(backend-test)
target_link_libraries(backend-test
    PRIVATE
        backend::backend
)
target_install_executable(backend-test)
