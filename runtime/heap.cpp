#include "runtime/heap.h"

void Heap::MarkFrom(GcNode* node) {
    if (!node || node->marked_) {
        return;
    }

    node->marked_ = true;
    node->Traverse([this](GcNode* child) { MarkFrom(child); });
}

void Heap::Collect(std::initializer_list<GcNode*> roots) {
    for (auto& node : nodes_) {
        node->marked_ = false;
    }

    for (auto* root : roots) {
        MarkFrom(root);
    }

    std::erase_if(nodes_, [](const auto& node) { return !node->marked_; });
}
