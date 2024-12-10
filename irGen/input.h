#ifndef JIT_AOT_COURSE_IR_GEN_INPUT
#define JIT_AOT_COURSE_IR_GEN_INPUT

namespace ir {

class SingleInstruction;

class Input {
  public:
    Input() : instr_(nullptr) {}
    Input(SingleInstruction *instr) : instr_(instr) {}
    Input(const Input &) = default;
    Input &operator=(const Input &) = default;
    Input(Input &&) = default;
    Input &operator=(Input &&) = default;
    ~Input() = default;

  public:
    SingleInstruction *GetInstruction() { return instr_; }
    const SingleInstruction *GetInstruction() const { return instr_; }
    void SetInstruction(SingleInstruction *newInstr) { instr_ = newInstr; }

  private:
    SingleInstruction *instr_;
};

inline bool operator==(const Input &lhs, const Input &rhs) {
    return lhs.GetInstruction() == rhs.GetInstruction();
}
inline bool operator==(const SingleInstruction *lhs, const Input &rhs) {
    return lhs == rhs.GetInstruction();
}
inline bool operator==(const Input &lhs, const SingleInstruction *rhs) {
    return lhs.GetInstruction() == rhs;
}

} // namespace ir

#endif // JIT_AOT_COURSE_IR_GEN_INPUT