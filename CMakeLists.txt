cmake_minimum_required(VERSION 3.29.3)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/rogue)
project(rogue)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD_REQUIRED true)

#
# common functions
#

function(target_enable_warnings target)
    set(gcc_api "$<COMPILE_LANG_AND_ID:C,ARMClang,AppleClang,Clang,GNU,LCC>")
    set(msvc_api "$<COMPILE_LANG_AND_ID:C,MSVC>")
    target_compile_options(${target}
        PRIVATE
            "$<${gcc_api}:-Wall;-Werror>"
            "$<${msvc_api}:-W3;-WX>"
    )
endfunction()

function(target_install_executable target)
    INSTALL(TARGETS ${target})
    INSTALL(FILES $<TARGET_RUNTIME_DLLS:${target}> TYPE BIN)
endfunction()

#
# backend
#

add_subdirectory(backend)

#
# backend-test
#

add_subdirectory(samples)

#
# c++ std module
#

# I was not able to get this working...
# add_subdirectory(cxx_std)

#
# core
#

add_subdirectory(core)

#
# resources
#

install(DIRECTORY resources DESTINATION .)

#
# compile_commands
#

# add_custom_target(reformat-compile-database
#     ALL
#     COMMAND cmake -E cat build/compile-commands.json | python scripts/reformat-compile-database.py > build/compile-commands.json
#     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#     COMMENT ${CMAKE_CURRENT_SOURCE_DIR}
# )
# add_dependencies(reformat-compile-database all)
