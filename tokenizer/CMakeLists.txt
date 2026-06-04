add_library(tokenizer
    tokenizer.cpp
)

target_link_libraries(tokenizer PUBLIC scheme_includes)
