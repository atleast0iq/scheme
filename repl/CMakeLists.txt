add_executable(scheme_repl
    main.cpp
)

target_link_libraries(scheme_repl PRIVATE interpreter)
