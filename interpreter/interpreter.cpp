#include "interpreter/interpreter.h"

#include "parser/parser.h"
#include "runtime/environment.h"
#include "runtime/error.h"
#include "runtime/object.h"
#include "tokenizer/tokenizer.h"

#include <sstream>

namespace {

void AssertNoTrailingTokens(Tokenizer& tokenizer) {
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("Unexpected token after expression");
    }
}

} // namespace

Interpreter& Interpreter::Instance() {
    static Interpreter interpreter;
    return interpreter;
}

Interpreter::Interpreter() : env_(MakeDefaultEnv()) {
}

Interpreter::~Interpreter() {
    Heap::Instance().Collect({});
}

void Interpreter::Reset() {
    env_ = MakeDefaultEnv();
    Heap::Instance().Collect({env_});
}

std::string Interpreter::Run(const std::string& input) {
    std::stringstream stream{input};
    Tokenizer tokenizer{&stream};
    auto* ast = Read(&tokenizer);
    AssertNoTrailingTokens(tokenizer);
    if (!ast) {
        throw RuntimeError("Cannot evaluate empty list");
    }

    auto* result = ast->Eval(*env_);
    std::string serialized = result ? result->Serialize() : "()";
    Heap::Instance().Collect({env_});
    return serialized;
}
