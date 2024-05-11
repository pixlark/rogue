cmake_minimum_required(VERSION 3.29.3)
project(backend)

set(SDL_ATOMIC_ENABLED_BY_DEFAULT     false)
set(SDL_AUDIO_ENABLED_BY_DEFAULT      false)
set(SDL_VIDEO_ENABLED_BY_DEFAULT      true)
set(SDL_RENDER_ENABLED_BY_DEFAULT     true)
set(SDL_EVENTS_ENABLED_BY_DEFAULT     true)
set(SDL_JOYSTICK_ENABLED_BY_DEFAULT   false)
set(SDL_HAPTIC_ENABLED_BY_DEFAULT     false)
set(SDL_HIDAPI_ENABLED_BY_DEFAULT     false)
set(SDL_POWER_ENABLED_BY_DEFAULT      false)
set(SDL_THREADS_ENABLED_BY_DEFAULT    true)
set(SDL_TIMERS_ENABLED_BY_DEFAULT     false)
set(SDL_FILE_ENABLED_BY_DEFAULT       false)
set(SDL_LOADSO_ENABLED_BY_DEFAULT     true)
set(SDL_CPUINFO_ENABLED_BY_DEFAULT    false)
set(SDL_FILESYSTEM_ENABLED_BY_DEFAULT true)
set(SDL_SENSOR_ENABLED_BY_DEFAULT     false)
set(SDL_LOCALE_ENABLED_BY_DEFAULT     false)
set(SDL_MISC_ENABLED_BY_DEFAULT       false)

add_subdirectory(SDL)
add_subdirectory(stb_image)

add_library(backend SHARED)
add_library(backend::backend ALIAS backend)

target_include_directories(backend
    INTERFACE
        .
)

set_target_properties(backend
    PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS true
)
target_sources(backend
    PRIVATE
        backend.c
)
target_enable_warnings(backend)

if(TARGET SDL2::SDL2main)
    target_link_libraries(backend PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(backend
    PRIVATE
        SDL2::SDL2
        stb_image::stb_image
)
