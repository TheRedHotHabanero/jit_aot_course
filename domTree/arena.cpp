#include "arena.h"
#include "sys/mman.h"
#include <cassert>

namespace memory {
ArenaAllocator::~ArenaAllocator() noexcept {
    assert(arenaList);
    while (arenaList) {
        assert(munmap(arenaList->start, arenaList->GetSize()) == 0);
        auto *next = arenaList->GetNextArena();
        delete arenaList;
        arenaList = next;
    }
}

void *Arena::Alloc(size_t sz, size_t alignSize) {
    assert(IsPowerOfTwo(alignSize));
    void *aligned = std::align(alignSize, sz, end, freeSize);
    if (aligned == nullptr) {
        return nullptr;
    }
    end = VoidPtrT(UintptrT(end) + sz);
    freeSize -= sz;
    return aligned;
}

void ArenaAllocator::addNewArena() {
    void *mem = mmap(nullptr, arenaSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(mem != MAP_FAILED);
    auto *newArena = new Arena(mem, arenaSize);
    assert(newArena);
    newArena->SetNextArena(arenaList);
    arenaList = newArena;
}

void *ArenaAllocator::allocate(size_t size) {
    auto addr = arenaList->Alloc(size, alignment);
    if (addr == nullptr) {
        if (size > arenaList->GetSize()) {
            arenaSize = AlignUp(size, PAGE_SIZE);
        }
        addNewArena();
        return allocate(size);
    }
    return addr;
}

STLCompliantArenaAllocator<int> ArenaAllocator::ToSTL() {
    return STLCompliantArenaAllocator<int>(this);
}
}   // namespace utils::memory