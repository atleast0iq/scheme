#pragma once

#include "runtime/builtins.h"
#include "runtime/object.h"

#include <array>

inline constexpr std::array<BuiltinDefinition, 25> kBuiltinFunctions = {{
    {"number?", MakeIsNumberFunction},  {"+", MakeAddFunction},
    {"-", MakeSubtractFunction},        {"*", MakeMultiplyFunction},
    {"/", MakeDivideFunction},          {"max", MakeMaxFunction},
    {"min", MakeMinFunction},           {"abs", MakeAbsFunction},
    {"=", MakeEqualFunction},           {"<", MakeLessFunction},
    {">", MakeGreaterFunction},         {"<=", MakeLessOrEqualFunction},
    {">=", MakeGreaterOrEqualFunction}, {"boolean?", MakeIsBooleanFunction},
    {"not", MakeNotFunction},           {"pair?", MakeIsPairFunction},
    {"null?", MakeIsNullFunction},      {"list?", MakeIsListFunction},
    {"cons", MakeConsFunction},         {"car", MakeCarFunction},
    {"cdr", MakeCdrFunction},           {"list", MakeListFunction},
    {"list-ref", MakeListRefFunction},  {"list-tail", MakeListTailFunction},
    {"symbol?", MakeIsSymbolFunction},
}};
