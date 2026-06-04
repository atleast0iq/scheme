#include "interpreter/interpreter.h"
#include "parser/parser.h"
#include "runtime/error.h"
#include "tokenizer/tokenizer.h"
#include "tests/fuzzer.h"

#include <gtest/gtest.h>
#include <sstream>

static constexpr uint32_t kShotsCount = 100000;

TEST(Fuzzing, ParserDoesNotCrash) {
    Fuzzer fuzzer;

    for (uint32_t i = 0; i < kShotsCount; ++i) {
        try {
            auto req = fuzzer.Next();
            std::stringstream ss{req};
            Tokenizer tokenizer{&ss};
            while (!tokenizer.IsEnd()) {
                Read(&tokenizer);
            }
        } catch (const SyntaxError&) {
        }
    }
}

TEST(Fuzzing, InterpreterDoesNotCrash) {
    Fuzzer fuzzer;
    Interpreter::Instance().Reset();

    for (uint32_t i = 0; i < kShotsCount; ++i) {
        try {
            Interpreter::Instance().Run(fuzzer.Next());
        } catch (const SyntaxError&) {
        } catch (const RuntimeError&) {
        } catch (const NameError&) {
        }
    }
}
