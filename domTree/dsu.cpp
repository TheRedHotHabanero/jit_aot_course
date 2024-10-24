#include "dsu.h"
#include <iostream>

namespace ir {

// Find operation in the DSU
BB *DSU::Find(BB *bblock) {
    // Validate the block and check for a valid ID
    if (bblock == nullptr || bblock->GetId() >= GetSize()) {
        std::cout << "[DSU Error] in Find" << std::endl;
        std::abort();
    }

    if (parentLinks_[bblock->GetId()] == nullptr) { // root
        return bblock;
    }

    // Update the ancestor path to find the representative
    UpdateAncestorPath(bblock);
    return labels_->at(bblock->GetId());
}

// Update the ancestor path to optimize future queries
void DSU::UpdateAncestorPath(BB *bblock) {
    // Get the ancestor of the current block
    auto ancestor = parentLinks_[bblock->GetId()];

    if (ancestor == nullptr) {
        std::cout << "[DSU Error] UpdateAncestorPath Universum" << std::endl;
        std::abort();
    }

    // If the ancestor is a root, stop the recursion
    if (parentLinks_[ancestor->GetId()] == nullptr) {
        return;
    }

    // Recursively update the ancestor path
    UpdateAncestorPath(ancestor);

    // Determine the minimum semi-dominator for the current block
    auto minForBBlock = labels_->at(bblock->GetId());
    auto minForAnc = labels_->at(ancestor->GetId());

    // Compare the semi-dominators to update the label if necessary
    if (sdoms_->at(minForAnc->GetId()) < sdoms_->at(minForBBlock->GetId())) {
        labels_->at(bblock->GetId()) =
            minForAnc; // Update label to the ancestor
    }

    // Path compression - connect the block to its ancestor's parent
    parentLinks_[bblock->GetId()] = parentLinks_[ancestor->GetId()];
}

} // namespace ir
