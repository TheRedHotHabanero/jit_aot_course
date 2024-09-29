/*
    Instruction structure is stolen from lecture

                                                   Basic Block
    ____________________________________________________________________________________________________________________
    | Instruction 1                Instruction 2                                                        Instruction n   |
    | - prev                       - prev           (intrusive linked list)                             - prev          |
    | - next                       - next           (intrusive linked list)          |       |          - next          |
    | - opcode           <-->      - opcode         (common part)              <---> | <---> | <--->    - opcode        |
    | - type                       - type           (common part)                    |       |          - type          |
    | - basic block                - basic block    (common part)                                       - basic block   |
    | - inputs                     - inputs         (derived part)                                      - inputs        |
    ____________________________________________________________________________________________________________________|



    Id              - uniq number of inst. If phi -> id contains "p"
    Type            - type of result of inst (s32, u64, ...)
    Opcode          - name of instruction
    Basic flags     - basic inst properties (throwable, no_dce, ...)
    Special flags   - special properties for inst (conditional code, inlined, ...)
    Inputs          - array of instructions, which need to execute the instruction
    Users           - list of instructions, which have instuction as input
*/

/*
    Graph structure is stolen from lecture

                        Graph
        __________________________________________________________________________
       | Blocks             |
       | Method Info        |   keeps shared data between compiler/interpreter
       | Runtime Info       |   get info about runtime types
       | Optimiztions Info  |   keeps info about analyses and optimization pipeline
       | Backend info       |   arch-specific |
   ___________________|______________________________________________________
*/

/*
    Basic block structure is stolen from lecture


    Basic block       Basic block       Basic block
         |                 |                 |
         |                 |                 |
         |____________Basic block____________|
                    ________________
                    | preds(many!) |
                    | succs(<=2)   |
                    | first phi    |
                    | first inst   |
                    | last inst    |
                    | graph        |
                    |______________|
                           |
                           |
                           |
                      Basic Block
                           |
                           |
    Basic Block <----------------------> Basic Block
*/