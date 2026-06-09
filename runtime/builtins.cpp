#include "runtime/builtins.h"

#include "runtime/error.h"
#include "runtime/object.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace {

void AssertExactArgumentCount(std::string_view builtin_name, const std::vector<Object*>& args,
                              std::size_t expected) {
    if (args.size() != expected) {
        throw RuntimeError(std::string(builtin_name) + ": wrong number of arguments");
    }
}

void AssertMinArgumentCount(std::string_view builtin_name, const std::vector<Object*>& args,
                            std::size_t minimum) {
    if (args.size() < minimum) {
        throw RuntimeError(std::string(builtin_name) + ": wrong number of arguments");
    }
}

int64_t ReadIntegerArgument(std::string_view builtin_name, Object* arg,
                            std::size_t argument_index) {
    if (auto* number = As<Number>(arg)) {
        return number->GetValue();
    }
    throw RuntimeError(std::string(builtin_name) + ": argument " +
                       std::to_string(argument_index + 1) + " must be a number");
}

template <class Operation>
int64_t ApplyFoldOperation(Operation operation, int64_t lhs, int64_t rhs,
                           std::size_t argument_index) {
    if constexpr (requires { operation(lhs, rhs, argument_index); }) {
        return operation(lhs, rhs, argument_index);
    } else {
        return operation(lhs, rhs);
    }
}

template <class Operation>
int64_t FoldArguments(std::string_view builtin_name, const std::vector<Object*>& args,
                      std::size_t first_index, int64_t initial, Operation operation) {
    int64_t result = initial;
    for (std::size_t i = first_index; i < args.size(); ++i) {
        result =
            ApplyFoldOperation(operation, result, ReadIntegerArgument(builtin_name, args[i], i), i);
    }
    return result;
}

template <class Operation>
Object* MakeVariadicIntegerFunction(Heap& heap, std::string name, int64_t initial,
                                    Operation operation) {
    return MakeBuiltinFunction(heap, name,
                               [&heap, name, initial, operation](const std::vector<Object*>& args,
                                                                 Environment&) -> Object* {
                                   return MakeNumber(
                                       heap, FoldArguments(name, args, 0, initial, operation));
                               });
}

template <class Operation>
Object* MakeNonEmptyFoldFunction(Heap& heap, std::string name, Operation operation) {
    return MakeBuiltinFunction(
        heap, name,
        [&heap, name, operation](const std::vector<Object*>& args, Environment&) -> Object* {
            AssertMinArgumentCount(name, args, 1);
            int64_t initial = ReadIntegerArgument(name, args.front(), 0);
            return MakeNumber(heap, FoldArguments(name, args, 1, initial, operation));
        });
}

template <class Predicate>
Object* MakeComparisonFunction(Heap& heap, std::string name, Predicate predicate) {
    return MakeBuiltinFunction(
        heap, name,
        [&heap, name, predicate](const std::vector<Object*>& args, Environment&) -> Object* {
            if (args.empty()) {
                return MakeBoolean(heap, true);
            }

            bool result = true;
            int64_t previous = ReadIntegerArgument(name, args.front(), 0);
            for (std::size_t i = 1; i < args.size(); ++i) {
                int64_t current = ReadIntegerArgument(name, args[i], i);
                result = result && predicate(previous, current);
                previous = current;
            }
            return MakeBoolean(heap, result);
        });
}

template <class Predicate>
Object* MakeUnaryBooleanFunction(Heap& heap, std::string name, Predicate predicate) {
    return MakeBuiltinFunction(
        heap, name,
        [&heap, name, predicate](const std::vector<Object*>& args, Environment&) -> Object* {
            AssertExactArgumentCount(name, args, 1);
            return MakeBoolean(heap, predicate(args.front()));
        });
}

bool IsProperList(Object* object) {
    auto* current = object;
    while (current) {
        auto* cell = As<Cell>(current);
        if (!cell) {
            return false;
        }
        current = cell->GetSecond();
    }
    return true;
}

Cell* ReadPairArgument(std::string_view builtin_name, Object* arg) {
    auto* cell = As<Cell>(arg);
    if (!cell) {
        throw RuntimeError(std::string(builtin_name) + ": argument must be a pair");
    }
    return cell;
}

std::size_t ReadListIndex(std::string_view builtin_name, Object* arg) {
    int64_t index = ReadIntegerArgument(builtin_name, arg, 1);
    if (index < 0) {
        throw RuntimeError(std::string(builtin_name) + ": index must be non-negative");
    }
    return static_cast<std::size_t>(index);
}

Object* BuildList(Heap& heap, const std::vector<Object*>& args) {
    Object* result = nullptr;
    for (auto it = args.rbegin(); it != args.rend(); ++it) {
        result = MakeCell(heap, *it, result);
    }
    return result;
}

Object* ListRef(Object* list, std::size_t index) {
    auto* current = list;
    for (std::size_t i = 0;; ++i) {
        auto* cell = As<Cell>(current);
        if (!cell) {
            throw RuntimeError("list-ref: index is out of range");
        }
        if (i == index) {
            return cell->GetFirst();
        }
        current = cell->GetSecond();
    }
}

Object* ListTail(Object* list, std::size_t index) {
    auto* current = list;
    for (std::size_t i = 0; i < index; ++i) {
        auto* cell = As<Cell>(current);
        if (!cell) {
            throw RuntimeError("list-tail: index is out of range");
        }
        current = cell->GetSecond();
    }
    return current;
}

} // namespace

Object* MakeIsNumberFunction(Heap& heap) {
    return MakeUnaryBooleanFunction(heap, "number?", [](Object* arg) { return Is<Number>(arg); });
}

Object* MakeAddFunction(Heap& heap) {
    return MakeVariadicIntegerFunction(heap, "+", 0,
                                       [](int64_t lhs, int64_t rhs) { return lhs + rhs; });
}

Object* MakeSubtractFunction(Heap& heap) {
    return MakeBuiltinFunction(
        heap, "-", [&heap](const std::vector<Object*>& args, Environment&) -> Object* {
            AssertMinArgumentCount("-", args, 1);
            int64_t initial = ReadIntegerArgument("-", args.front(), 0);
            if (args.size() == 1) {
                return MakeNumber(heap, -initial);
            }
            return MakeNumber(heap,
                              FoldArguments("-", args, 1, initial,
                                            [](int64_t lhs, int64_t rhs) { return lhs - rhs; }));
        });
}

Object* MakeMultiplyFunction(Heap& heap) {
    return MakeVariadicIntegerFunction(heap, "*", 1,
                                       [](int64_t lhs, int64_t rhs) { return lhs * rhs; });
}

Object* MakeDivideFunction(Heap& heap) {
    return MakeBuiltinFunction(
        heap, "/", [&heap](const std::vector<Object*>& args, Environment&) -> Object* {
            AssertMinArgumentCount("/", args, 2);
            int64_t initial = ReadIntegerArgument("/", args.front(), 0);
            return MakeNumber(
                heap, FoldArguments("/", args, 1, initial,
                                    [](int64_t lhs, int64_t divisor, std::size_t index) {
                                        if (divisor == 0) {
                                            throw RuntimeError("/: division by zero in argument " +
                                                               std::to_string(index + 1));
                                        }
                                        return lhs / divisor;
                                    }));
        });
}

Object* MakeModuloFunction(Heap& heap) {
    return MakeBuiltinFunction(
        heap, "%", [&heap](const std::vector<Object*>& args, Environment&) -> Object* {
            AssertExactArgumentCount("%", args, 2);
            return MakeNumber(heap, 
                ReadIntegerArgument("%", args[0], 0) %
                ReadIntegerArgument("%", args[1], 1));
        });
}

Object* MakeMaxFunction(Heap& heap) {
    return MakeNonEmptyFoldFunction(heap, "max",
                                    [](int64_t lhs, int64_t rhs) { return std::max(lhs, rhs); });
}

Object* MakeMinFunction(Heap& heap) {
    return MakeNonEmptyFoldFunction(heap, "min",
                                    [](int64_t lhs, int64_t rhs) { return std::min(lhs, rhs); });
}

Object* MakeAbsFunction(Heap& heap) {
    return MakeBuiltinFunction(heap, "abs",
                               [&heap](const std::vector<Object*>& args, Environment&) -> Object* {
                                   AssertExactArgumentCount("abs", args, 1);
                                   int64_t value = ReadIntegerArgument("abs", args.front(), 0);
                                   return MakeNumber(heap, value < 0 ? -value : value);
                               });
}

Object* MakeEqualFunction(Heap& heap) {
    return MakeComparisonFunction(heap, "=", [](int64_t lhs, int64_t rhs) { return lhs == rhs; });
}

Object* MakeLessFunction(Heap& heap) {
    return MakeComparisonFunction(heap, "<", [](int64_t lhs, int64_t rhs) { return lhs < rhs; });
}

Object* MakeGreaterFunction(Heap& heap) {
    return MakeComparisonFunction(heap, ">", [](int64_t lhs, int64_t rhs) { return lhs > rhs; });
}

Object* MakeLessOrEqualFunction(Heap& heap) {
    return MakeComparisonFunction(heap, "<=", [](int64_t lhs, int64_t rhs) { return lhs <= rhs; });
}

Object* MakeGreaterOrEqualFunction(Heap& heap) {
    return MakeComparisonFunction(heap, ">=", [](int64_t lhs, int64_t rhs) { return lhs >= rhs; });
}

Object* MakeIsBooleanFunction(Heap& heap) {
    return MakeUnaryBooleanFunction(heap, "boolean?", [](Object* arg) { return Is<Boolean>(arg); });
}

Object* MakeNotFunction(Heap& heap) {
    return MakeBuiltinFunction(heap, "not",
                               [&heap](const std::vector<Object*>& args, Environment&) -> Object* {
                                   AssertExactArgumentCount("not", args, 1);
                                   return MakeBoolean(heap, IsFalse(args.front()));
                               });
}

Object* MakeIsPairFunction(Heap& heap) {
    return MakeUnaryBooleanFunction(heap, "pair?", [](Object* arg) { return Is<Cell>(arg); });
}

Object* MakeIsNullFunction(Heap& heap) {
    return MakeUnaryBooleanFunction(heap, "null?", [](Object* arg) { return arg == nullptr; });
}

Object* MakeIsListFunction(Heap& heap) {
    return MakeUnaryBooleanFunction(heap, "list?", [](Object* arg) { return IsProperList(arg); });
}

Object* MakeConsFunction(Heap& heap) {
    return MakeBuiltinFunction(heap, "cons",
                               [&heap](const std::vector<Object*>& args, Environment&) -> Object* {
                                   AssertExactArgumentCount("cons", args, 2);
                                   return MakeCell(heap, args[0], args[1]);
                               });
}

Object* MakeCarFunction(Heap& heap) {
    return MakeBuiltinFunction(heap, "car",
                               [](const std::vector<Object*>& args, Environment&) -> Object* {
                                   AssertExactArgumentCount("car", args, 1);
                                   return ReadPairArgument("car", args.front())->GetFirst();
                               });
}

Object* MakeCdrFunction(Heap& heap) {
    return MakeBuiltinFunction(heap, "cdr",
                               [](const std::vector<Object*>& args, Environment&) -> Object* {
                                   AssertExactArgumentCount("cdr", args, 1);
                                   return ReadPairArgument("cdr", args.front())->GetSecond();
                               });
}

Object* MakeListFunction(Heap& heap) {
    return MakeBuiltinFunction(heap, "list",
                               [&heap](const std::vector<Object*>& args, Environment&) -> Object* {
                                   return BuildList(heap, args);
                               });
}

Object* MakeListRefFunction(Heap& heap) {
    return MakeBuiltinFunction(heap, "list-ref",
                               [](const std::vector<Object*>& args, Environment&) -> Object* {
                                   AssertExactArgumentCount("list-ref", args, 2);
                                   return ListRef(args.front(), ReadListIndex("list-ref", args[1]));
                               });
}

Object* MakeListTailFunction(Heap& heap) {
    return MakeBuiltinFunction(
        heap, "list-tail", [](const std::vector<Object*>& args, Environment&) -> Object* {
            AssertExactArgumentCount("list-tail", args, 2);
            return ListTail(args.front(), ReadListIndex("list-tail", args[1]));
        });
}

Object* MakeIsSymbolFunction(Heap& heap) {
    return MakeUnaryBooleanFunction(heap, "symbol?", [](Object* arg) { return Is<Symbol>(arg); });
}
