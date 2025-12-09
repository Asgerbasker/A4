#include <stdint.h>
#include <stdio.h>

// Uses a bitmask to return 7 bits from bit 0-6 of the instruction - the operation code
static uint32_t get_opcode(uint32_t inst) {
    return inst & 0x7F;
}

// Uses a bitmask to return 5 bits from bit 7-11 of the instruction - the destination register
static uint32_t get_rd(uint32_t inst) {
    return inst >> 7 & 0x1F;
}

// Uses a bitmask to return 5 bits from bit 15-19 of the instruction - the first source register
static uint32_t get_rs1(uint32_t inst) {
    return inst >> 15 & 0x1F;
}

// Uses a bitmask to return 5 bits form bit 20-24 of the instruction - the second source register (or shamt)
static uint32_t get_rs2(uint32_t inst) {
    return inst >> 20 & 0x1F;
}

// Get immediate (or offset)
static int32_t get_imm(uint32_t inst) {
    int32_t imm = inst >> 20;             // imm[11:0] = inst[31:20]
    return (imm << 20) >> 20;             // sign extend 12-bit immediate (imm[11] is the sign bit)
}   

// Get store immediate 
static int32_t get_store_imm(uint32_t inst) {
    int32_t imm = 0;
    imm |= (inst >> 7) & 0x1F;            // imm[4:0] = inst[11:7]
    imm |= ((inst >> 25) & 0x7F) << 5;    // imm[11:5] = inst[31:25]
    return (imm << 20) >> 20;             // sign-extend 12-bit immediate (imm[11] is the sign bit)
}

// Get upper immediate
static int32_t get_upper_imm(uint32_t inst) {
    int32_t imm = (inst >> 12) & 0xFFFFF; // imm[31:12] = inst[31:12]
    return imm << 12;
}

// Get branch immediate
static int32_t get_branch_imm(uint32_t inst) {
    int32_t imm = 0;
    imm |= ((inst >> 7) & 0x1) << 11;     // imm[11] = inst[7]
    imm |= ((inst >> 8) & 0xF) << 1;      // imm[4:1] = inst[11:8] 
    imm |= ((inst >> 25) & 0x3F) << 5;    // imm[10:5] = inst[30:25]
    imm |= ((inst >> 31) & 0x1) << 12;    // imm[12] = inst[31]   
    return (imm << 19) >> 19;             // sign-extend 13-bit immediate (imm[12] is the sign bit)
}

// Get jump immediate 
static int32_t get_jump_imm(uint32_t inst) {
    int32_t imm = 0;
    imm |= ((inst >> 21) & 0x3FF) << 1;   // imm[10:1] = inst[30:21]
    imm |= ((inst >> 20) & 0x1) << 11;    // imm[11] = inst[20]
    imm |= ((inst >> 12) & 0xFF) << 12;   // imm[19:12] = inst[19:12]
    imm |= ((inst >> 31) & 0x1) << 20;    // imm[20] = inst[31]
    return (imm << 11) >> 11;             // sign-extend 21-bit immediate (imm[20] is the sign bit)
}

// Uses bitmask to return 3 bits from bits 12-14 - the funct3
static uint32_t get_funct3(uint32_t inst) {
    return inst >> 12 & 0x7;
}

// Uses bitmask to return 7 bits from bits 25-31 - the funct7
static uint32_t get_funct7(uint32_t inst) {
    return inst >> 25 & 0x7F;
}

// Write register, but ignore x0
static inline void write_reg(uint32_t rd, int32_t value, int32_t registers[]) {
    if (rd != 0 && rd < 32) {
        registers[rd] = value;
    }
}

// RV32M: helper functions for multiplication/division
static int32_t rv32m_mul(int32_t a, int32_t b) {
    int64_t p = (int64_t)a * (int64_t)b;
    return (int32_t)p;    // lower 32 bit
}

static int32_t rv32m_mulh(int32_t a, int32_t b) {
    int64_t p = (int64_t)a * (int64_t)b;
    return (int32_t)(p >> 32);   // upper 32 bit
}

static int32_t rv32m_mulhsu(int32_t a, uint32_t b) {
    int64_t p = (int64_t)a * (int64_t)b;
    return (int32_t)(p >> 32);
}

static int32_t rv32m_mulhu(uint32_t a, uint32_t b) {
    uint64_t p = (uint64_t)a * (uint64_t)b;
    return (int32_t)(p >> 32);
}

static int32_t rv32m_div(int32_t a, int32_t b) {
    if (b == 0) {
        return -1;
    }
    if (a == INT32_MIN && b == -1) {
        return INT32_MIN; // special overflow-case
    }
    return a / b;
}

static uint32_t rv32m_divu(uint32_t a, uint32_t b) {
    if (b == 0) {
        return 0xFFFFFFFFu;
    }
    return a / b;
}

static int32_t rv32m_rem(int32_t a, int32_t b) {
    if (b == 0) {
        return a;
    }
    if (a == INT32_MIN && b == -1) return 0;
    return a % b;
}

static uint32_t rv32m_remu(uint32_t a, uint32_t b) {
    if (b == 0) {
        return a;
    }
    return a % b;
}

// Syscall: ecall with A7 (x17) as call-number
// 1: getchar() -> A0
// 2: putchar(A0)
// 3 and 93: end program
static int handle_syscall(int32_t registers[]) {
    int32_t a7 = registers[17];
    int32_t a0 = registers[10];

    switch (a7) {
        case 1: { // getchar
            int c = getchar();
            if (c == EOF) {
                c = -1;
            }
            write_reg(10, c, registers);
            break;
        }
        case 2: { // putchar
            putchar((unsigned char)a0);
            fflush(stdout);
            break;
        }
        case 3: 
        case 93: {
            // end simulation
            return 0;   // 0 = stop
        }
        default: {
            // unknown syscall: stop
            fprintf(stderr, "Unknown syscall: %d\n", a7);
            return 0;
        }
    }
    return 1; // 1 = continue 
}