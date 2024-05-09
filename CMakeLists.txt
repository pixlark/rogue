cmake_minimum_required(VERSION 3.29.3)
project(rogue)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_subdirectory(backend)

add_executable(backend-test)
target_sources(backend-test
    PRIVATE
        "./backend-test.c"
)
target_link_libraries(backend-test
    PUBLIC
        backend::backend
)

if (WIN32)
    add_custom_command(
        TARGET backend-test
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:backend-test> $<TARGET_RUNTIME_DLLS:backend-test>
        COMMAND_EXPAND_LISTS
    )
endif()
