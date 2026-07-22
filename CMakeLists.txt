cmake_minimum_required(VERSION 3.24)

include(version.cmake)
set(package_name bnet)

project(${package_name} VERSION ${build_version} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")
set(CMAKE_BUILD_RPATH "$ORIGIN")

add_subdirectory(lib)

if(${PROJECT_IS_TOP_LEVEL})
    add_subdirectory(example)
endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type
        $<$<NOT:$<CONFIG:Debug>>:-Werror>
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        $<$<NOT:$<CONFIG:Debug>>:/WX>
    )
endif()