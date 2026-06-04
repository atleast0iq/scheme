cmake_minimum_required(VERSION 3.16)

project(scheme LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(scheme_includes INTERFACE)
target_include_directories(scheme_includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

add_subdirectory(tokenizer)
add_subdirectory(runtime)
add_subdirectory(parser)
add_subdirectory(interpreter)
add_subdirectory(repl)
add_subdirectory(tests)
