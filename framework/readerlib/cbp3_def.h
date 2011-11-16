// Author: Hongliang Gao;   Created: Jan 27 2011
// Description: common definitions and helper functions for cbp3.

#ifndef CBP3_DEF_H_INCLUDED
#define CBP3_DEF_H_INCLUDED

#define NUM_REGS 121
#define PIPE_WIDTH 4
// each uop has up to 6 src registers
// function reg_is_constant can be used to tell whether a src is a real register
// or a constant value
#define UOP_SRCS_MAX 6
// the framework provides timing info of six stages:
// fetch, allocation, execution, agu, std (store data is ready) and retire
#define NUM_STAGES 6

// uop type encoding
#define IS_BR_CONDITIONAL (1 << 0)   // conditional branch
#define IS_BR_INDIRECT    (1 << 1)   // indirect branch
#define IS_BR_CALL        (1 << 2)   // call
#define IS_BR_RETURN      (1 << 3)   // return
#define IS_BR_OTHER       (1 << 4)   // other branches
#define IS_LOAD           (1 << 5)   // load uop
#define IS_STORE          (1 << 6)   // store uop
#define IS_FP             (1 << 7)   // floating point uop
#define IS_WFLAGS         (1 << 8)   // uop writes flags
#define IS_RFLAGS         (1 << 9)   // uop reads flags
#define IS_BOM            (1 << 14)  // uop is the beginning uop of an instruction
#define IS_EOM            (1 << 15)  // uop is the end uop of an instruction

// register type
typedef enum {
    REG_ZERO,  // constant zero
    REG_ONES,  // constant ones
    REG_IMM,   // immediate value
    REG_INT,   // integer register
    REG_SEG,   // segment register
    REG_FP,    // floating point register
    REG_OTHER  // control register etc.
}REG_TYPE;

// some of register names that may be useful:
//             Register Name      Register Number
//                  RSP                7
//                  RBP                8
//                  RSI                9
//                  RDI               10

// some helper functions
REG_TYPE get_reg_type(uint8_t r);     // convert register number to type
bool     reg_is_constant(uint8_t r);  // check whether a register is REG_ZERO, REG_ONES or REG_IMM
bool     uop_is_branch(int16_t type); // check whether an uop is a branch

// dynamic and static info of an uop
class cbp3_uop_dynamic_t {
    public:
        cbp3_uop_dynamic_t (){ reset(); }
        void reset();
        
        // dynamic info
        // NOTE: srcs and dst may be different from srcs_static and dst_static due to some stack registers
        uint32_t srcs[UOP_SRCS_MAX];         // src registers generated at allocations stage
        uint32_t dst;                        // dst register generated at allocation stage

        uint32_t src_data[UOP_SRCS_MAX];     // values of src registers
        uint32_t dst_data;                   // value of dst register

        // flag register has 5 bits
        // corresponding flags are:
        // bit0: CF, bit1: ZF, bit2: OF, bit3: SF, bit4: PF
        uint32_t wflags;                    // flag register value after written by this uop
        uint32_t rflags;                    // flag register value used by this uop (before its execution)

        // load/store can have up to 16 bytes values
        uint32_t ldst_data[4];              // load/store values, value size (in bytes) is in opsize field
        uint32_t vaddr;                     // virtual address of load/store

        uint32_t br_target;                 // real branch target
        bool     br_taken;                  // real branch direction

        // static info

        // uop_id is an unique id used in trace reader
        // it is reset to 0 in the framework and not supposed to be used by predictors
        uint32_t uop_id;

        uint8_t  inst_size;                 // size of an instruction (in bytes)
        uint32_t pc;                        // address of an instruction
        uint16_t type;                      // uop type ( refer to IS_* encodings defined above)
        uint8_t  opsize;                    // op size (in bytes)
        uint8_t  srcs_static[UOP_SRCS_MAX]; // static src registers
        uint8_t  dst_static;                // static dst register
};

#endif
