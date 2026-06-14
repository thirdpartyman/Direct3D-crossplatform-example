cmake_minimum_required(VERSION 3.24)
project(SDL3_D3D9_DXVK_Example C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(WIN32)
    if(MSVC)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")       
    else()
        string(APPEND CMAKE_EXE_LINKER_FLAGS " -static-libgcc")
        string(APPEND CMAKE_SHARED_LINKER_FLAGS " -static-libgcc")
    endif()
endif()

include(FetchContent)

find_package(SDL3 QUIET)
if(SDL3_FOUND)
    set(SDL3_TARGET SDL3::SDL3)
else()
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY    https://github.com/libsdl-org/SDL
        GIT_TAG           release-3.2.0
        GIT_SHALLOW       TRUE
        GIT_PROGRESS TRUE
    )
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL3)
    set(SDL3_TARGET SDL3-shared)
endif()

if(WIN32)
    set(GRAPHICS_LIBRARIES "d3d9")
    set(GRAPHICS_INCLUDE_DIRS "")
else()
    find_path(DXVK_INCLUDE_DIR NAMES d3d9.h PATH_SUFFIXES dxvk)
    find_library(DXVK_D3D9_LIB NAMES dxvk_d3d9 dxvk d3d9)

    if(NOT DXVK_D3D9_LIB OR NOT DXVK_INCLUDE_DIR)
        message(FATAL_ERROR "DXVK Native headers or libraries not found! Please install libdxvk via system package manager.")
    endif()

    set(GRAPHICS_LIBRARIES ${DXVK_D3D9_LIB})
    set(GRAPHICS_INCLUDE_DIRS ${DXVK_INCLUDE_DIR})
endif()

add_executable(sdl3_d3d9_dxvk_example main.c)

if(GRAPHICS_INCLUDE_DIRS)
    target_include_directories(sdl3_d3d9_dxvk_example PRIVATE ${GRAPHICS_INCLUDE_DIRS})
endif()

target_link_libraries(sdl3_d3d9_dxvk_example PRIVATE 
    ${SDL3_TARGET}
    ${GRAPHICS_LIBRARIES}
)

set_target_properties(sdl3_d3d9_dxvk_example PROPERTIES WIN32_EXECUTABLE TRUE)

add_custom_command(TARGET sdl3_d3d9_dxvk_example POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	$<TARGET_FILE:${SDL3_TARGET}>
	$<TARGET_FILE_DIR:sdl3_d3d9_dxvk_example>
)
