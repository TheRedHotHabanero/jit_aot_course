#ifndef JIT_AOT_COURSE_USER_H_
#define JIT_AOT_COURSE_USER_H_

#include "domTree/arena.h"
#include <cassert>
#include <span>

namespace ir {
class SingleInstruction;

class User {
  public:
    explicit User(memory::ArenaAllocator *const allocator)
        : users_(allocator->ToSTL()) {
        assert(allocator);
    }
    User(std::span<SingleInstruction *> instrs,
         memory::ArenaAllocator *const allocator)
        : users_(instrs.begin(), instrs.end(), allocator->ToSTL()) {
        assert(allocator);
    }
    User(const User &) = delete;
    User &operator=(const User &) = delete;

    User(User &&) = delete;
    User &operator=(User &&) = delete;

    virtual ~User() = default;

    const memory::ArenaVector<SingleInstruction *> &GetUsers() const {
        return users_;
    }
    std::span<SingleInstruction *> GetUsers() { return std::span(users_); }

    void AddUser(SingleInstruction *instr) {
        assert(instr);
        users_.push_back(instr);
    }
    void AddUsers(std::span<SingleInstruction *> instrs) {
        users_.reserve(users_.size() + instrs.size());
        for (auto &&it : instrs) {
            users_.push_back(it);
        }
    }

    void RemoveUser(SingleInstruction *instr) {
        auto iter = std::find(users_.begin(), users_.end(), instr);
        assert(iter != users_.end());
        *iter = users_.back();
        users_.pop_back();
    }

    void ReplaceUser(SingleInstruction *oldInstr, SingleInstruction *newInstr) {
        auto iter = std::find(users_.begin(), users_.end(), oldInstr);
        assert(iter != users_.end());
        *iter = newInstr;
    }

    size_t UsersCount() const { return users_.size(); }

    void SetNewUsers(memory::ArenaVector<SingleInstruction *> &&newUsers) {
        users_ = std::move(newUsers);
    }

  protected:
    memory::ArenaVector<SingleInstruction *> users_;
};
} // namespace ir

#endif // JIT_AOT_COURSE_USER_H_