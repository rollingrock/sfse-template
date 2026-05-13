cmake_minimum_required(VERSION 3.25)

project(
    sfse-template-plugin
    VERSION 0.1.0
    DESCRIPTION "Hello-world Starfield Script Extender plugin template"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(NOT CMAKE_GENERATOR_PLATFORM AND NOT CMAKE_VS_PLATFORM_NAME AND NOT CMAKE_GENERATOR MATCHES "Win64|x64")
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "sfse-template-plugin requires a 64-bit toolchain (x64).")
    endif()
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

include(FetchSFSE)

add_library(${PROJECT_NAME} SHARED
    src/PCH.h
    src/Plugin.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

target_link_libraries(${PROJECT_NAME} PRIVATE sfse::sfse_common)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /MP /W4 /permissive- /Zc:__cplusplus /Zc:preprocessor /EHsc
    )
endif()

find_package(spdlog CONFIG QUIET)
if(spdlog_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SFSE_TEMPLATE_HAVE_SPDLOG)
    message(STATUS "sfse-template-plugin: spdlog found - logging via spdlog")
else()
    message(STATUS "sfse-template-plugin: spdlog not found - falling back to std::ofstream logging")
endif()

include(DeployPlugin)
sfse_template_deploy_plugin(${PROJECT_NAME})
