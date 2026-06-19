cmake_minimum_required(VERSION 3.24)
project(SDL3_D3D9_DXVK_Example C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    if(MSVC)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")       
    else()
        string(APPEND CMAKE_EXE_LINKER_FLAGS " -static-libgcc -static-libstdc++")
        string(APPEND CMAKE_SHARED_LINKER_FLAGS " -static-libgcc -static-libstdc++")
    endif()
endif()

include(FetchContent)

set(FETCHCONTENT_QUIET OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SDL_GPU OFF CACHE BOOL "" FORCE)
set(SDL_RENDER OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)

set(SDL_OPENGL OFF CACHE BOOL "" FORCE)
set(SDL_OPENGLES OFF CACHE BOOL "" FORCE)
set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
set(SDL_RENDER_D3D OFF CACHE BOOL "" FORCE)
set(SDL_RENDER_D3D11 OFF CACHE BOOL "" FORCE)
set(SDL_RENDER_D3D12 OFF CACHE BOOL "" FORCE)
set(SDL_RENDER_GPU OFF CACHE BOOL "" FORCE)
set(SDL_RENDER_VULKAN OFF CACHE BOOL "" FORCE)

set(SDL_DUMMYAUDIO OFF CACHE BOOL "" FORCE)
set(SDL_DUMMYCAMERA OFF CACHE BOOL "" FORCE)
set(SDL_DUMMYVIDEO OFF CACHE BOOL "" FORCE)
set(SDL_OFFSCREEN OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SDL3
    GIT_REPOSITORY    https://github.com/libsdl-org/SDL
    GIT_TAG           release-3.2.0
    GIT_SHALLOW       TRUE
    GIT_PROGRESS      TRUE
    FIND_PACKAGE_ARGS QUIET
)

FetchContent_MakeAvailable(SDL3)

if(TARGET SDL3::SDL3)
    set(SDL3_TARGET SDL3::SDL3)
else()
    set(SDL3_TARGET SDL3-shared)
endif()

if(WIN32)
    set(GRAPHICS_LIBRARIES "d3d9" "d3dcompiler")
    set(GRAPHICS_INCLUDE_DIRS "")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(DXVK_D3D9 REQUIRED dxvk-d3d9)
    pkg_check_modules(VKD3D_UTILS REQUIRED libvkd3d-utils)

    set(GRAPHICS_LIBRARIES ${DXVK_D3D9_LIBRARIES} ${VKD3D_UTILS_LIBRARIES})
    set(GRAPHICS_INCLUDE_DIRS ${DXVK_D3D9_INCLUDE_DIRS} ${VKD3D_UTILS_INCLUDE_DIRS})
endif()

set(PROJECT_SOURCES main.cpp)
if(NOT WIN32)
    list(APPEND PROJECT_SOURCES shader_compiler.c)
endif()

add_executable(sdl3_d3d9_dxvk_example ${PROJECT_SOURCES})

if(GRAPHICS_INCLUDE_DIRS)
    target_include_directories(sdl3_d3d9_dxvk_example PRIVATE ${GRAPHICS_INCLUDE_DIRS})
endif()

target_link_libraries(sdl3_d3d9_dxvk_example PRIVATE 
    ${SDL3_TARGET}
    ${GRAPHICS_LIBRARIES}
)

set_target_properties(sdl3_d3d9_dxvk_example PROPERTIES WIN32_EXECUTABLE TRUE)

if(TARGET ${SDL3_TARGET})
    add_custom_command(TARGET sdl3_d3d9_dxvk_example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${SDL3_TARGET}>
        $<TARGET_FILE_DIR:sdl3_d3d9_dxvk_example>
    )
endif()

