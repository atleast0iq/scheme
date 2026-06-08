#include "runtime/environment.h"

#include "runtime/error.h"
#include "runtime/forms.h"
#include "runtime/functions.h"
#include "runtime/object.h"

namespace {

template <class Definitions>
void RegisterBindings(Environment* env, Heap& heap, const Definitions& definitions) {
    for (const auto& [name, factory] : definitions) {
        env->Define(name, factory(heap));
    }
}

} // namespace

Environment::Environment(Environment* parent) : parent_(parent) {
}

void Environment::Define(const std::string& name, Object* value) {
    values_[name] = value;
}

void Environment::Set(const std::string& name, Object* value) {
    if (auto it = values_.find(name); it != values_.end()) {
        it->second = value;
        return;
    }
    if (parent_) {
        parent_->Set(name, value);
        return;
    }
    throw NameError("Unknown symbol");
}

Object* Environment::Lookup(const std::string& name) const {
    if (auto it = values_.find(name); it != values_.end()) {
        return it->second;
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
    for (const auto& [name, value] : values_) {
        if (value) {
            visit(value);
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
