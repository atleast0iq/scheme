#pragma once

#include "runtime/object.h"
#include "runtime/special_forms.h"

#include <array>

inline constexpr std::array<BuiltinDefinition, 9> kBuiltinForms = {{
    {"quote", MakeQuoteForm},
    {"and", MakeAndForm},
    {"or", MakeOrForm},
    {"if", MakeIfForm},
    {"define", MakeDefineForm},
    {"set!", MakeSetForm},
    {"lambda", MakeLambdaForm},
    {"set-car!", MakeSetCarForm},
    {"set-cdr!", MakeSetCdrForm},
}};
