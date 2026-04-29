#pragma once

#include <concepts>
#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

class GcNode {
public:
    virtual ~GcNode() = default;
    virtual void Traverse(const std::function<void(GcNode*)>& visit) = 0;

private:
    friend class Heap;
    bool marked_ = false;
};

class Heap {
public:
    static Heap& Instance() {
        static Heap heap;
        return heap;
    }

    template <std::derived_from<GcNode> T, typename... Args> T* Create(Args&&... args) {
        auto object = std::make_unique<T>(std::forward<Args>(args)...);
        auto* raw = object.get();
        nodes_.push_back(std::move(object));
        return raw;
    }

    void Collect(std::initializer_list<GcNode*> roots);

private:
    void MarkFrom(GcNode* node);

    std::vector<std::unique_ptr<GcNode>> nodes_;
};
