cmake_minimum_required(VERSION 3.16)

project(scheme LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(scheme
    tokenizer/tokenizer.cpp
    parser/parser.cpp
    interpreter/interpreter.cpp
    runtime/heap.cpp
    runtime/object.cpp
    runtime/environment.cpp
    runtime/special_forms.cpp
    runtime/builtins.cpp
)

target_include_directories(scheme PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(scheme_repl
    repl/main.cpp
)

target_link_libraries(scheme_repl PRIVATE
    scheme
)
