#include "runtime/environment.h"

#include "runtime/error.h"
#include "runtime/forms.h"
#include "runtime/functions.h"
#include "runtime/object.h"

#include <algorithm>

namespace {

constexpr size_t kEnvironmentReserve = 64;

using Binding = std::pair<std::string, Object*>;

template <class Definitions>
void RegisterBindings(Environment* env, Heap& heap, const Definitions& definitions) {
    for (const auto& [name, factory] : definitions) {
        env->Define(name, factory(heap));
    }
}

auto FindBinding(auto& values, const std::string& name) {
    return std::ranges::find(values, name, &Binding::first);
}

} // namespace

Environment::Environment(Environment* parent) : parent_(parent) {
    values_.reserve(kEnvironmentReserve);
}

void Environment::Define(const std::string& name, Object* value) {
    if (auto binding = FindBinding(values_, name); binding != values_.end()) {
        binding->second = value;
        return;
    }
    values_.emplace_back(name, value);
}

void Environment::Set(const std::string& name, Object* value) {
    if (auto binding = FindBinding(values_, name); binding != values_.end()) {
        binding->second = value;
        return;
    }
    if (parent_) {
        parent_->Set(name, value);
        return;
    }
    throw NameError("Unknown symbol");
}

Object* Environment::Lookup(const std::string& name) const {
    if (auto binding = FindBinding(values_, name); binding != values_.end()) {
        return binding->second;
    }
    if (parent_) {
        return parent_->Lookup(name);
    }
    throw NameError("Unknown symbol");
}

void Environment::Traverse(const std::function<void(GcNode*)>& visit) {
    if (parent_) {
        visit(parent_);
    }
    for (const auto& binding : values_) {
        if (binding.second) {
            visit(binding.second);
        }
    }
}

Environment* MakeDefaultEnv() {
    auto& heap = Heap::Instance();
    auto* env = heap.Create<Environment>();
    RegisterBindings(env, heap, kBuiltinForms);
    RegisterBindings(env, heap, kBuiltinFunctions);
    return env;
}
