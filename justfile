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

test: debug
    ./{{debug_dir}}/tests/scheme_tests

coverage_dir := "build/coverage"

coverage:
    cmake -S . -B {{coverage_dir}} -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
    cmake --build {{coverage_dir}}
    ./{{coverage_dir}}/tests/scheme_tests
    lcov --capture --directory {{coverage_dir}} --output-file {{coverage_dir}}/coverage.info --exclude '/usr/*' --exclude '*/tests/*'
    genhtml {{coverage_dir}}/coverage.info --output-directory {{coverage_dir}}/html
    @echo "Report: {{coverage_dir}}/html/index.html"

clean:
    rm -rf build
