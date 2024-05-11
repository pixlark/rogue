cmake_minimum_required(VERSION 3.29.3)
project(core)

add_executable(core)

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
        core.cc
)
target_compile_features(core
    PUBLIC
        cxx_std_23
)
target_enable_warnings(core)
target_compile_options(core PRIVATE /analyze /analyze:plugin EspXEngine.dll)
target_link_libraries(core
    PRIVATE
        backend::backend
)
target_install_executable(core)
