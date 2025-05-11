#ifndef JIT_AOT_COURSE_ARENA_H_
#define JIT_AOT_COURSE_ARENA_H_

#include <cassert>
#include <memory>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace memory {

constexpr size_t DEFAULT_ALIGNMENT = alignof(uintptr_t);

inline constexpr size_t AlignUp(size_t addr, size_t page_size) {
    return ((addr + (page_size - 1)) / page_size) * page_size;
}

inline constexpr bool IsPowerOfTwo(size_t val) {
    return (val & (val - 1)) == 0;
}

inline uintptr_t UintptrT(void *addr) {
    return reinterpret_cast<uintptr_t>(addr);
}

inline void *VoidPtrT(uintptr_t addr) { return reinterpret_cast<void *>(addr); }

class Arena {
  public:
    Arena(void *mem, size_t size)
        : size(size), freeSize(size), start(mem), end(mem) {
        assert(mem);
    }

    // Copy and move semantics
    Arena(const Arena &) = default;
    Arena &operator=(const Arena &) = default;
    Arena(Arena &&) = default;
    Arena &operator=(Arena &&) = default;
    virtual ~Arena() = default;

    size_t GetSize() const { return size; }
    size_t GetFreeSize() const { return freeSize; }
    Arena *GetNextArena() { return nextArena; }
    const Arena *GetNextArena() const { return nextArena; }

    void SetNextArena(Arena *arena) { nextArena = arena; }

    [[nodiscard]] void *Alloc(size_t sz, size_t alignSize);

  private:
    size_t size;
    size_t freeSize;
    void *start;
    void *end;
    Arena *nextArena = nullptr;

    friend class ArenaAllocator;
};

// Utility trait
template <typename T>
using NonVoid = std::enable_if_t<!std::is_same<T, void>::value>;

template <typename T> class STLCompliantArenaAllocator;

template <typename T>
using ArenaVector = std::vector<T, STLCompliantArenaAllocator<T>>;

template <typename T>
using ArenaSet = std::set<T, std::less<T>, STLCompliantArenaAllocator<T>>;

template <typename KeyT, typename ValueT>
using ArenaUnorderedMap = std::unordered_map<
    KeyT, ValueT, std::hash<KeyT>, std::equal_to<KeyT>,
    STLCompliantArenaAllocator<std::pair<const KeyT, ValueT>>>;

class ArenaAllocator final {
  public:
    explicit ArenaAllocator(size_t arenaSize = DEFAULT_ARENA_SIZE,
                            size_t alignment = DEFAULT_ALIGNMENT)
        : arenaList(nullptr), arenaSize(AlignUp(arenaSize, PAGE_SIZE)),
          alignment(alignment) {
        addNewArena();
    }

    // No copy semantics
    ArenaAllocator(const ArenaAllocator &) = delete;
    ArenaAllocator &operator=(const ArenaAllocator &) = delete;

    // Default move semantics
    ArenaAllocator(ArenaAllocator &&) = default;
    ArenaAllocator &operator=(ArenaAllocator &&) = default;

    ~ArenaAllocator() noexcept;
    size_t GetFreeSize() const { return arenaList->GetFreeSize(); }
    STLCompliantArenaAllocator<int> ToSTL();

    template <typename T> [[nodiscard]] T *AllocateArray(size_t n) {
        auto p = allocate(sizeof(T) * n);
        if (p == nullptr) {
            return nullptr;
        }
        return static_cast<T *>(p);
    }

    template <typename T, typename... ArgsT>
    [[nodiscard]] T *New(ArgsT &&...args) {
        auto p = allocate(sizeof(T));
        if (p == nullptr) {
            return nullptr;
        }
        new (p) T(std::forward<ArgsT>(args)...);
        return reinterpret_cast<T *>(p);
    }

    template <typename KeyT, typename ValueT, typename... ArgsT>
    [[nodiscard]] inline ArenaUnorderedMap<KeyT, ValueT> *
    NewUnorderedMap(ArgsT &&...args);

    template <typename T, typename... ArgsT>
    [[nodiscard]] inline ArenaVector<T> *NewVector(ArgsT &&...args);

    static constexpr size_t DEFAULT_ARENA_SIZE = 4096;

  private:
    void addNewArena();
    void *allocate(size_t size);

    Arena *arenaList;
    size_t arenaSize;
    size_t alignment;

    static constexpr size_t PAGE_SIZE = 4096;
};

template <typename T> class STLCompliantArenaAllocator {
  public:
    using pointer = T *;
    using value_type = T;
    using size_type = size_t;
    using difference_type = size_t;

    explicit STLCompliantArenaAllocator(ArenaAllocator *const alloc)
        : allocator(alloc) {
        assert(allocator);
    }
    STLCompliantArenaAllocator() : allocator(nullptr) {}

    template <typename V>
    STLCompliantArenaAllocator(const STLCompliantArenaAllocator<V> &other)
        : allocator(other.GetAllocator()) {}

    STLCompliantArenaAllocator(const STLCompliantArenaAllocator &) noexcept =
        default;
    STLCompliantArenaAllocator &
    operator=(const STLCompliantArenaAllocator &) noexcept = default;
    STLCompliantArenaAllocator(STLCompliantArenaAllocator &&) noexcept =
        default;
    STLCompliantArenaAllocator &
    operator=(STLCompliantArenaAllocator &&) noexcept = default;
    virtual ~STLCompliantArenaAllocator() noexcept = default;

    [[nodiscard]] pointer allocate(size_type n) {
        return allocator->template AllocateArray<T>(n);
    }

    void deallocate([[maybe_unused]] pointer p, [[maybe_unused]] size_type n) {}

    ArenaAllocator *GetAllocator() const { return allocator; }

    template <typename U>
    bool operator==(const STLCompliantArenaAllocator<U> &other) const noexcept {
        return allocator == other.allocator;
    }

    template <typename U>
    bool operator!=(const STLCompliantArenaAllocator<U> &other) const noexcept {
        return !(operator==(other));
    }

  private:
    ArenaAllocator *const allocator;
};

template <typename T>
inline bool operator==(const STLCompliantArenaAllocator<T> &lhs,
                       const STLCompliantArenaAllocator<T> &rhs) {
    return lhs.GetAllocator() == rhs.GetAllocator();
}

template <typename T, typename... ArgsT>
[[nodiscard]] inline ArenaVector<T> *
ArenaAllocator::NewVector(ArgsT &&...args) {
    return New<ArenaVector<T>>(std::forward<ArgsT>(args)..., ToSTL());
}

template <typename KeyT, typename ValueT, typename... ArgsT>
[[nodiscard]] inline ArenaUnorderedMap<KeyT, ValueT> *
ArenaAllocator::NewUnorderedMap(ArgsT &&...args) {
    return New<ArenaUnorderedMap<KeyT, ValueT>>(std::forward<ArgsT>(args)...,
                                                ToSTL());
}

} // namespace memory

#endif // JIT_AOT_COURSE_ARENA_H_
