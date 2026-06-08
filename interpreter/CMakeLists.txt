add_library(interpreter interpreter.cpp)

    target_link_libraries(interpreter PUBLIC scheme_includes runtime parser)
