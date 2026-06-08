add_library(runtime heap.cpp object.cpp environment.cpp special_forms.cpp builtins.cpp)

    target_link_libraries(runtime PUBLIC scheme_includes)
