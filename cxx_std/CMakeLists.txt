cmake_minimum_required(VERSION 3.29.3)
project(cxx_std)

#
# std module for Ninja+MSVC
#

# if ((CMAKE_GENERATOR STREQUAL "Ninja") AND (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
#     set(VC_TOOLS_INSTALL_DIR "$ENV{VCToolsInstallDir}")
#     if (VC_TOOLS_INSTALL_DIR STREQUAL "")
#         message(FATAL_ERROR "No MSVC environment! (%VCToolsInstallDir% missing)")
#     endif()

#     add_library(cxx_std STATIC)
#     add_library(cxx_std::cxx_std ALIAS cxx_std)

#     target_sources(cxx_std
#         PRIVATE
#             "${VC_TOOLS_INSTALL_DIR}/modules/std.ixx"
#     )

#     target_compile_features(cxx_std PUBLIC cxx_std_23)

#     target_compile_options(cxx_std
#         INTERFACE
#             "/reference \"std=std.ifc\"")
# endif()

#
# utility functions for module support
#

function(target_enable_cxx_modules target)
target_compile_features(${target} PUBLIC cxx_std_23)

    # this used to be a thing but maybe not anymore on VS? https://gitlab.kitware.com/cmake/cmake/-/issues/24922
    # set_target_properties(${target} PROPERTIES VS_USER_PROPS "msvc/enable-modules.props")

    if ((CMAKE_GENERATOR STREQUAL "Ninja") AND (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
        set(VC_TOOLS_INSTALL_DIR "$ENV{VCToolsInstallDir}")
        if (VC_TOOLS_INSTALL_DIR STREQUAL "")
            message(FATAL_ERROR "No MSVC environment! (%VCToolsInstallDir% missing)")
        endif()

        target_sources(${target}
            PRIVATE
                FILE_SET CXX_MODULES
                BASE_DIRS
                    ${CMAKE_CURRENT_SOURCE_DIR} "${VC_TOOLS_INSTALL_DIR}/modules"
                FILES
                    "${VC_TOOLS_INSTALL_DIR}/modules/std.ixx"
        )

        # target_link_libraries(${target}
        #     PRIVATE
        #         cxx_std::cxx_std
        # )
    endif()
endfunction()

macro(target_cxx_module_sources target privacy)
    target_sources(${target}
        ${privacy}
            FILE_SET CXX_MODULES
            FILES
                ${ARGN}
    )
endmacro()
