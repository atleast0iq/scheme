#include "runtime/special_forms.h"

#include "runtime/environment.h"
#include "runtime/error.h"
#include "runtime/object.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

void AssertProperRawArgumentList(std::string_view form_name, Object* raw_args) {
    auto* current = raw_args;
    while (current != nullptr) {
        if (!Is<Cell>(current)) {
            throw SyntaxError(std::string(form_name) + " arguments must form a proper list");
        }
        current = As<Cell>(current)->GetSecond();
    }
}

Cell* ReadSingleArgumentCell(std::string_view form_name, Object* raw_args,
                             const char* error_message) {
    AssertProperRawArgumentList(form_name, raw_args);
    auto* args = As<Cell>(raw_args);
    if (!args || args->GetSecond() != nullptr) {
        throw SyntaxError(error_message);
    }
    return args;
}

std::pair<Object*, Object*> ReadTwoArguments(std::string_view form_name, Object* raw_args,
                                             const char* error_message) {
    AssertProperRawArgumentList(form_name, raw_args);
    auto* first_cell = As<Cell>(raw_args);
    auto* second_cell = first_cell ? As<Cell>(first_cell->GetSecond()) : nullptr;
    if (!first_cell || !second_cell || second_cell->GetSecond() != nullptr) {
        throw SyntaxError(error_message);
    }
    return {first_cell->GetFirst(), second_cell->GetFirst()};
}

std::vector<std::string> ReadParameterNames(Object* params) {
    std::vector<std::string> result;
    auto* current = params;
    while (current != nullptr) {
        auto* cell = As<Cell>(current);
        if (!cell) {
            throw SyntaxError("lambda parameters must form a proper list");
        }
        auto* symbol = As<Symbol>(cell->GetFirst());
        if (!symbol) {
            throw SyntaxError("lambda parameters must be symbols");
        }
        result.push_back(symbol->GetName());
        current = cell->GetSecond();
    }
    return result;
}

Cell* RequirePairValue(const std::string& form_name, Object* object) {
    auto* cell = As<Cell>(object);
    if (!cell) {
        throw RuntimeError(form_name + ": argument must be a pair");
    }
    return cell;
}

Object* MakeLambdaValue(Heap& heap, Environment& env, Object* params, Object* body) {
    if (!body) {
        throw SyntaxError("lambda expects parameter list and body");
    }
    return heap.Create<LambdaFunction>(ReadParameterNames(params), body, &env);
}

Object* MakeShortCircuitForm(Heap& heap, std::string name, bool empty_result, auto should_return) {
    return MakeBuiltinForm(
        heap, name,
        [&heap, name, empty_result, should_return](Object* raw_args, Environment& env) -> Object* {
            AssertProperRawArgumentList(name, raw_args);

            Object* result = MakeBoolean(heap, empty_result);
            for (auto* current = raw_args; current; current = As<Cell>(current)->GetSecond()) {
                auto* expression = As<Cell>(current)->GetFirst();
                AssertExpressionExists("argument", expression);
                result = expression->Eval(env);
                if (should_return(result)) {
                    return result;
                }
            }
            return result;
        });
}

} // namespace

Object* MakeQuoteForm(Heap& heap) {
    return MakeBuiltinForm(heap, "quote", [](Object* raw_args, Environment&) -> Object* {
        return ReadSingleArgumentCell("quote", raw_args, "quote expects exactly one argument")
            ->GetFirst();
    });
}

Object* MakeAndForm(Heap& heap) {
    return MakeShortCircuitForm(heap, "and", true, IsFalse);
}

Object* MakeOrForm(Heap& heap) {
    return MakeShortCircuitForm(heap, "or", false, [](Object* value) { return !IsFalse(value); });
}

Object* MakeIfForm(Heap& heap) {
    return MakeBuiltinForm(heap, "if", [&heap](Object* raw_args, Environment& env) -> Object* {
        AssertProperRawArgumentList("if", raw_args);

        auto* condition = As<Cell>(raw_args);
        if (!condition) {
            throw SyntaxError("if expects two or three arguments");
        }

        auto* consequent = As<Cell>(condition->GetSecond());
        if (!consequent) {
            throw SyntaxError("if expects two or three arguments");
        }

        auto* alternative_tail = consequent->GetSecond();
        Object* alternative = nullptr;
        if (alternative_tail) {
            auto* alternative_cell = As<Cell>(alternative_tail);
            if (!alternative_cell || alternative_cell->GetSecond()) {
                throw SyntaxError("if expects two or three arguments");
            }
            alternative = alternative_cell->GetFirst();
        }

        auto* condition_expression = condition->GetFirst();
        AssertExpressionExists("expression", condition_expression);
        if (IsFalse(condition_expression->Eval(env))) {
            return alternative ? heap.Create<TailCall>(alternative, &env) : nullptr;
        }

        auto* consequent_expression = consequent->GetFirst();
        AssertExpressionExists("expression", consequent_expression);
        return heap.Create<TailCall>(consequent_expression, &env);
    });
}

Object* MakeDefineForm(Heap& heap) {
    return MakeBuiltinForm(heap, "define", [&heap](Object* raw_args, Environment& env) -> Object* {
        AssertProperRawArgumentList("define", raw_args);
        auto* target_cell = As<Cell>(raw_args);
        if (!target_cell) {
            throw SyntaxError("define expects a name and value");
        }

        auto* target = target_cell->GetFirst();
        auto* rest = target_cell->GetSecond();
        auto* value_cell = As<Cell>(rest);
        if (!value_cell) {
            throw SyntaxError("define expects a name and value");
        }

        if (auto* symbol = As<Symbol>(target)) {
            if (value_cell->GetSecond() != nullptr) {
                throw SyntaxError("define expects a name and value");
            }
            auto* expression = value_cell->GetFirst();
            AssertExpressionExists("expression", expression);
            env.Define(symbol->GetName(), expression->Eval(env));
            return nullptr;
        }

        auto* signature = As<Cell>(target);
        if (!signature) {
            throw SyntaxError("define expects a symbol or function signature");
        }
        auto* name_symbol = As<Symbol>(signature->GetFirst());
        if (!name_symbol) {
            throw SyntaxError("define function name must be a symbol");
        }
        env.Define(name_symbol->GetName(),
                   MakeLambdaValue(heap, env, signature->GetSecond(), rest));
        return nullptr;
    });
}

Object* MakeSetForm(Heap& heap) {
    return MakeBuiltinForm(heap, "set!", [](Object* raw_args, Environment& env) -> Object* {
        auto [target, expression] =
            ReadTwoArguments("set!", raw_args, "set! expects a name and value");
        auto* symbol = As<Symbol>(target);
        if (!symbol) {
            throw SyntaxError("set! expects a symbol");
        }

        AssertExpressionExists("expression", expression);
        env.Set(symbol->GetName(), expression->Eval(env));
        return nullptr;
    });
}

Object* MakeLambdaForm(Heap& heap) {
    return MakeBuiltinForm(heap, "lambda", [&heap](Object* raw_args, Environment& env) -> Object* {
        AssertProperRawArgumentList("lambda", raw_args);
        auto* params_cell = As<Cell>(raw_args);
        if (!params_cell) {
            throw SyntaxError("lambda expects parameter list and body");
        }
        return MakeLambdaValue(heap, env, params_cell->GetFirst(), params_cell->GetSecond());
    });
}

Object* MakeSetCarForm(Heap& heap) {
    return MakeBuiltinForm(heap, "set-car!", [](Object* raw_args, Environment& env) -> Object* {
        auto [pair_expr, value_expr] =
            ReadTwoArguments("set-car!", raw_args, "set-car! expects two arguments");
        AssertExpressionExists("expression", pair_expr);
        AssertExpressionExists("expression", value_expr);
        auto* pair = RequirePairValue("set-car!", pair_expr->Eval(env));
        pair->SetFirst(value_expr->Eval(env));
        return nullptr;
    });
}

Object* MakeSetCdrForm(Heap& heap) {
    return MakeBuiltinForm(heap, "set-cdr!", [](Object* raw_args, Environment& env) -> Object* {
        auto [pair_expr, value_expr] =
            ReadTwoArguments("set-cdr!", raw_args, "set-cdr! expects two arguments");
        AssertExpressionExists("expression", pair_expr);
        AssertExpressionExists("expression", value_expr);
        auto* pair = RequirePairValue("set-cdr!", pair_expr->Eval(env));
        pair->SetSecond(value_expr->Eval(env));
        return nullptr;
    });
}
