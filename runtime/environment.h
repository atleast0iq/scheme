#pragma once

#include "runtime/heap.h"

#include <functional>
#include <string>
#include <unordered_map>

class Object;

class Environment : public GcNode {
public:
    explicit Environment(Environment* parent = nullptr);

    void Define(const std::string& name, Object* value);
    void Set(const std::string& name, Object* value);
    Object* Lookup(const std::string& name) const;

    void Traverse(const std::function<void(GcNode*)>& visit) override;

private:
    Environment* parent_;
    std::unordered_map<std::string, Object*> values_;
};

Environment* MakeDefaultEnv();
