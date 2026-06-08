add_library(parser parser.cpp)

    target_link_libraries(parser PUBLIC scheme_includes runtime tokenizer)
