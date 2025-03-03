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

    if (getParentLinks(bblock->GetId()) == nullptr) { // root
        return bblock;
    }

    // Update the ancestor path to find the representative
    UpdateAncestorPath(bblock);
    return labels_->at(bblock->GetId());
}

// Update the ancestor path to optimize future queries
void DSU::UpdateAncestorPath(BB *bblock) {
    // Get the ancestor of the current block
    auto ancestor = getParentLinks(bblock->GetId());

    if (ancestor == nullptr) {
        std::cout << "[DSU Error] UpdateAncestorPath Universum" << std::endl;
        std::abort();
    }

    // If the ancestor is a root, stop the recursion
    if (getParentLinks(ancestor->GetId()) == nullptr) {
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
    setParentLinks(bblock->GetId(), getParentLinks(ancestor->GetId()));
}

void DSU::Dump() {
    std::cout << "--- DSU ---\n";
    for (size_t i = 0; i < GetSize(); ++i) {
        if (getParentLinks(i) != nullptr) {
            std::cout << "= BB " << i << ": " << getParentLinks(i)->GetId() << std::endl;
        } else {
            std::cout << "= BB " << i << " is root\n";
        }
    }
    std::cout << "-----------" << std::endl;
}

} // namespace ir
