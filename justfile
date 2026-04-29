set dotenv-load := false

debug_dir := "build/debug"
release_dir := "build/release"

default: run-release

format:
    clang-format -i interpreter/* parser/* repl/* runtime/* tokenizer/*

compile-commands:
    cmake -S . -B {{debug_dir}} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cp {{debug_dir}}/compile_commands.json compile_commands.json

debug:
    cmake -S . -B {{debug_dir}} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build {{debug_dir}}

release:
    cmake -S . -B {{release_dir}} -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build {{release_dir}}

run-debug: debug
    ./{{debug_dir}}/scheme_repl

run-release: release
    ./{{release_dir}}/scheme_repl

clean:
    rm -rf build
