#include "runtime/object.h"

#include "runtime/environment.h"
#include "runtime/error.h"

#include <string>
#include <vector>

namespace {

void AppendSerialized(std::string& output, Object* object) {
    output += object ? object->Serialize() : "()";
}

void AssertProperArgumentListSyntax(Object* object) {
    if (!Is<Cell>(object)) {
        throw SyntaxError("Function call arguments must form a proper list");
    }
}

std::vector<Object*> EvalArguments(Object* args, Environment& env) {
    std::vector<Object*> result;
    auto* current = args;

    while (current != nullptr) {
        AssertProperArgumentListSyntax(current);
        auto* cell = As<Cell>(current);
        auto* argument = cell->GetFirst();
        AssertExpressionExists("argument", argument);
        result.push_back(argument->Eval(env));
        current = cell->GetSecond();
    }

    return result;
}

} // namespace

Object* Symbol::Eval(Environment& env) {
    return env.Lookup(name_);
}

Object* Cell::Eval(Environment& env) {
    if (!first_) {
        throw RuntimeError("Cannot call empty list");
    }

    auto* callee = first_->Eval(env);
    if (auto* special_form = As<SpecialForm>(callee)) {
        return special_form->Apply(second_, env);
    }
    if (auto* function = As<Function>(callee)) {
        return function->Apply(EvalArguments(second_, env), env);
    }
    throw RuntimeError("Object is not callable");
}

std::string Cell::Serialize() const {
    std::string result = "(";
    auto* current = this;
    bool need_separator = false;

    while (true) {
        if (need_separator) {
            result += " ";
        }

        AppendSerialized(result, current->GetFirst());

        auto* tail = current->GetSecond();
        if (!tail) {
            result += ")";
            return result;
        }

        auto* next = As<Cell>(tail);
        if (!next) {
            result += " . ";
            AppendSerialized(result, tail);
            result += ")";
            return result;
        }

        current = next;
        need_separator = true;
    }
}

void Cell::Traverse(const std::function<void(GcNode*)>& visit) {
    if (first_) {
        visit(first_);
    }
    if (second_) {
        visit(second_);
    }
}

Object* LambdaFunction::Apply(const std::vector<Object*>& args, Environment&) {
    if (args.size() != params_.size()) {
        throw RuntimeError("Wrong number of arguments");
    }

    auto& heap = Heap::Instance();
    auto* local_env = heap.Create<Environment>(closure_);
    for (size_t i = 0; i < params_.size(); ++i) {
        local_env->Define(params_[i], args[i]);
    }

    Object* result = nullptr;
    auto* current = body_;
    while (current != nullptr) {
        auto* cell = As<Cell>(current);
        if (!cell) {
            throw SyntaxError("lambda body must form a proper list");
        }
        AssertExpressionExists("lambda body", cell->GetFirst());
        result = cell->GetFirst()->Eval(*local_env);
        current = cell->GetSecond();
    }
    return result;
}

void LambdaFunction::Traverse(const std::function<void(GcNode*)>& visit) {
    if (body_) {
        visit(body_);
    }
    if (closure_) {
        visit(closure_);
    }
}

Object* MakeBuiltinForm(Heap& heap, std::string name, RawArgsHandler handler) {
    return heap.Create<SpecialForm>(std::move(name), std::move(handler));
}

Object* MakeBuiltinFunction(Heap& heap, std::string name, EvaluatedArgsHandler handler) {
    return heap.Create<BuiltinFunction>(std::move(name), std::move(handler));
}

Object* MakeBoolean(Heap& heap, bool value) {
    return heap.Create<Boolean>(value);
}

Object* MakeNumber(Heap& heap, int64_t value) {
    return heap.Create<Number>(value);
}

Object* MakeSymbol(Heap& heap, std::string name) {
    return heap.Create<Symbol>(std::move(name));
}

Object* MakeCell(Heap& heap, Object* first, Object* second) {
    return heap.Create<Cell>(first, second);
}

bool IsFalse(Object* object) {
    auto* boolean = As<Boolean>(object);
    return boolean != nullptr && !boolean->GetValue();
}

void AssertExpressionExists(std::string_view context, Object* object) {
    if (!object) {
        throw RuntimeError(std::string("Cannot evaluate empty ") + std::string(context));
    }
}
