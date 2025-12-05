#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "disassemble.h"

// Uses a bitmask to return 7 bits from bit 0-6 of the instruction - the operation code
uint32_t get_opcode(uint32_t inst) {
    return inst & 0x7F;
}

// Uses a bitmask to return 5 bits from bit 7-11 of the instruction - the destination register
uint32_t get_rd(uint32_t inst) {
    return inst >> 7 & 0x1F;
}

// Uses a bitmask to return 5 bits from bit 15-19 of the instruction - the first source register
uint32_t get_rs1(uint32_t inst) {
    return inst >> 15 & 0x1F;
}

// Uses a bitmask to return 5 bits form bit 20-24 of the instruction - the second source register
uint32_t get_rs2(uint32_t inst) {
    return inst >> 20 & 0x1F;
}

// Uses bitmask to return 32 bits, sign extended, from bit 20-31 of the instruction - the immediate
int32_t get_imm(uint32_t inst) {
    int32_t imm = inst >> 20; // take bits 20-31
    return (imm << 20) >> 20; // sign extend from 12 bit to 32 bit
}   

// Uses bitmask to return 20 bits from bit 12-31 of the instruction - the upper immediate 
int32_t get_upper_imm(uint32_t inst) {
    int_fast32_t uimm = inst >> 12 & 0xFFFFF;
    return uimm << 12;
}

// Uses bitmask to return 3 bits from bits 12-14 - the funct3 
uint32_t get_funct3(uint32_t inst) {
    return inst >> 12 & 0x7;
}

// Uses bitmask to return 7 bits from bits 25-31 - the funct7
uint32_t get_funct7(uint32_t inst) {
    return inst >> 25 & 0x7F;
}

void disassemble(uint32_t addr, uint32_t instruction, char* result, size_t buf_size, struct symbols* symbols) {
    uint32_t op_code = get_opcode(instruction);
    uint32_t funct3 = get_funct3(instruction);
    uint32_t funct7 = get_funct7(instruction);
    uint32_t rd = get_rd(instruction);
    uint32_t rs1 = get_rs1(instruction);
    uint32_t rs2 = get_rs2(instruction);
    int32_t imm = get_imm(instruction);
    int32_t uimm = get_upper_imm(instruction);
    uint32_t shamt = get_rs2(instruction); // same bit placement as rs2

    switch (op_code) {
    /* I-type ALU (immediate) */
        case 0x13: {
            if (funct3 == 0x0) { 
                snprintf(result, buf_size, "addi x%d, x%d, %d", rd, rs1, imm);
            } else if (funct3 == 0x2) {
                snprintf(result, buf_size, "slti x%d, x%d, %d", rd, rs1, imm);
            } else if (funct3 == 0x3) {
                snprintf(result, buf_size, "sltiu x%d, x%d, %d", rd, rs1, imm);
            }else if (funct3 == 0x4) {
                snprintf(result, buf_size, "xori x%d, x%d, %d", rd, rs1, imm);
            } else if (funct3 == 0x6) {
                snprintf(result, buf_size, "ori x%d, x%d, %d", rd, rs1, imm);
            } else if (funct3 == 0x7) {
                snprintf(result, buf_size, "andi x%d, x%d, %d", rd, rs1, imm);
            } else if (funct3 == 0x1) {
                snprintf(result, buf_size, "slli x%d, x%d, %d", rd, rs1, shamt);
            } else if (funct3 == 0x5 && funct7 == 0x00) {
                snprintf(result, buf_size, "srli x%d, x%d, %d", rd, rs1, shamt);
            } else if (funct3 == 0x5 && funct7 == 0x20) {
                snprintf(result, buf_size, "srai x%d, x%d, %d", rd, rs1, shamt);
            } else {
                snprintf(result, buf_size, "unknown (0x%08x)", instruction);
            }
            break;
        }
    /* U-type ALU (upper immediate) */
        case 0x37: { 
            snprintf(result, buf_size, "lui x%d, %d", rd, uimm);
            break;
        }
        case 0x17: {
            snprintf(result, buf_size, "auipc x%d, %d", rd, uimm);
            break;
        }
    /* R-type ALU (register to register) */
        case 0x33: { 
            if (funct3 == 0x0 && funct7 == 0x00) { 
                snprintf(result, buf_size, "add x%d, x%d, x%d", rd, rs1, rs2);
            } else if (funct3 == 0x0 && funct7 == 0x20) { 
                snprintf(result, buf_size, "sub x%d, x%d, x%d", rd, rs1, rs2);
            } else if (funct3 == 0x1) {
                snprintf(result, buf_size, "sll x%d, x%d, x%d", rd, rs1, rs2);
            } else if (funct3 == 0x2) {
                snprintf(result, buf_size, "slt x%d, x%d, x%d", rd, rs1, rs2);
            } else if (funct3 == 0x3) {
                snprintf(result, buf_size, "sltu x%d, x%d, x%d", rd, rs1, rs2);
            } else if (funct3 == 0x4) {
                snprintf(result, buf_size, "xor x%d, x%d, x%d", rd, rs1, rs2);
            } else if (funct3 == 0x5 && funct7 == 0x00) {
                snprintf(result, buf_size, "srl x%d, x%d, x%d", rd, rs1, rs2);
            } else if (funct3 == 0x5 && funct7 == 0x20) {
                snprintf(result, buf_size, "sra x%d, x%d, x%d", rd, rs1, rs2);
            } else if (funct3 == 0x6) {
                snprintf(result, buf_size, "or x%d, x%d, x%d", rd, rs1, rs2);
            } else if (funct3 == 0x7) {
                snprintf(result, buf_size, "and x%d, x%d, x%d", rd, rs1, rs2);
            } else {
                snprintf(result, buf_size, "unknown (0x%08x)", instruction);
            }
            break;
        }
        case 0x03: {
            if (funct3 == 0x0) {
                snprintf(result, buf_size,"lb x%d, x%d ,%d", rd, rs1, imm);
            } else {
                snprintf(result, buf_size, "unknown (0x%08x)", instruction);
            }
            break;
        }
        default:
            snprintf(result, buf_size, "unknown (0x%08x)", instruction);
            break;
        
    }
}