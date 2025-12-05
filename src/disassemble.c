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
int32_t get_imm_s(uint32_t inst){
    int32_t imm11_5 = (inst >> 25) & 0x7F;
    int32_t imm4_0  = (inst >> 7)  & 0x1F;
    int32_t imm = (imm11_5 << 5) | imm4_0;
    return (imm << 20) >> 20;   // sign-extend 12 bits
}

// Uses bitmask to return 20 bits from bit 12-31 of the instruction - the upper immediate
int32_t get_upper_imm(uint32_t inst) {
    int_fast32_t uimm = inst >> 12 & 0xFFFFF;
    return uimm << 12;
}

// Get branch immediate
int32_t get_branch_imm(uint32_t inst) {
    int32_t imm = 0;
    imm |= ((inst >> 7) & 0x1) << 11;     // imm[11]
    imm |= ((inst >> 8) & 0xF) << 1;      // imm[4:1]
    imm |= ((inst >> 25) & 0x3F) << 5;    // imm[10:5]
    imm |= ((inst >> 31) & 0x1) << 12;    // imm[12]
    // Sign-extend 13-bit immediate
    return (imm << 19) >> 19;
}

// Get jump immediate 
int32_t get_jump_imm(uint32_t inst) {
    int32_t imm = 0;
    imm |= ((inst >> 21) & 0x3FF) << 1; // imm[10:1] = inst[30:21]
    imm |= ((inst >> 20) & 0x1) << 11;  // imm[11] = inst[20]
    imm |= ((inst >> 12) & 0xFF) << 12; // imm[19:12] = inst[19:12]
    imm |= ((inst >> 31) & 0x1) << 20;  // imm[20] = inst[31]
    // Sign-extend 21 bit to 32 bit
    return (imm << 11) >> 11;
}

// Uses bitmask to return 3 bits from bits 12-14 - the funct3
uint32_t get_funct3(uint32_t inst) {
    return inst >> 12 & 0x7;
}

// Uses bitmask to return 7 bits from bits 25-31 - the funct7
uint32_t get_funct7(uint32_t inst) {
    return inst >> 25 & 0x7F;
}
uint32_t get_funct12(uint32_t inst) {
    int32_t imm = inst >> 20; // take bits 20-31
    return (imm << 20) >> 20; // sign extend from 12 bit to 32 bit
}

void disassemble(uint32_t addr, uint32_t instruction, char *result, size_t buf_size, struct symbols *symbols) {
    uint32_t op_code = get_opcode(instruction);
    uint32_t funct3 = get_funct3(instruction);
    uint32_t funct7 = get_funct7(instruction);
    uint32_t funct12 = get_funct12(instruction);
    uint32_t rd = get_rd(instruction);
    uint32_t rs1 = get_rs1(instruction);
    uint32_t rs2 = get_rs2(instruction);
    int32_t imm = get_imm(instruction);
    int32_t uimm = get_upper_imm(instruction);
    int32_t simm = get_imm_s(instruction);
    int32_t bimm = get_branch_imm(instruction);
    int32_t jimm = get_jump_imm(instruction);
    uint32_t shamt = get_rs2(instruction); // same bit placement as rs2

    switch (op_code) {
    /* I-type ALU (immediate) */
        case 0x13: {
            switch (funct3) {
                case 0x0: {// addi
                    snprintf(result, buf_size, "addi x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x2: {// slti
                    snprintf(result, buf_size, "slti x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x3: {// sltiu
                    snprintf(result, buf_size, "sltiu x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x4: {// xori
                    snprintf(result, buf_size, "xori x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x6: {// ori
                    snprintf(result, buf_size, "ori x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x7: {// andi
                    snprintf(result, buf_size, "andi x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x1: {// slli
                    snprintf(result, buf_size, "slli x%d, x%d, %d", rd, rs1, shamt);
                    break;
                }
                case 0x5: {
                    if (funct7 == 0x00) { // srli
                        snprintf(result, buf_size, "srli x%d, x%d, %d", rd, rs1, shamt);
                    } else if (funct7 == 0x20) { // srai
                        snprintf(result, buf_size, "srai x%d, x%d, %d", rd, rs1, shamt);
                    } else {
                        snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                    }
                    break;
                }
                default: {
                    snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                    break;
                }
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
    /* R-type ALU (register to register, including M extension) */
        case 0x33: {
            switch (funct7) {
                case 0x00: { // Standard ALU
                    switch (funct3) {
                        case 0x0: {
                            snprintf(result, buf_size, "add x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x1: {
                            snprintf(result, buf_size, "sll x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x2: {
                            snprintf(result, buf_size, "slt x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x3: {
                            snprintf(result, buf_size, "sltu x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x4: {
                            snprintf(result, buf_size, "xor x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x5: {
                            snprintf(result, buf_size, "srl x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x6: {
                            snprintf(result, buf_size, "or x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x7: {
                            snprintf(result, buf_size, "and x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        default: {
                            snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                            break;
                        }
                    }
                    break;
                }
                case 0x20: { // sub / sra
                    switch (funct3) {
                        case 0x0: {
                            snprintf(result, buf_size, "sub x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x5: {
                            snprintf(result, buf_size, "sra x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        default: {
                            snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                            break;
                        }
                    }
                    break;
                }
                case 0x01: { // M-extension
                    switch (funct3) {
                        case 0x0: {
                            snprintf(result, buf_size, "mul x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x1: {
                            snprintf(result, buf_size, "mulh x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x2: {
                            snprintf(result, buf_size, "mulhsu x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x3: {
                            snprintf(result, buf_size, "mulhu x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x4: {
                            snprintf(result, buf_size, "div x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x5: {
                            snprintf(result, buf_size, "divu x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x6: {
                            snprintf(result, buf_size, "rem x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        case 0x7: {
                            snprintf(result, buf_size, "remu x%d, x%d, x%d", rd, rs1, rs2);
                            break;
                        }
                        default: {
                            snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                            break;
                        }
                    }
                    break;
                }
                default: {
                    snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                    break;
                    }
            }
            break;
        }
        case 0x03: {
            if (funct3 == 0x0) {
                snprintf(result, buf_size,"lb x%d, %d(x%d)", rd, imm, rs1);
            } else if(funct3==0x1){
                snprintf(result,buf_size,"lh x%d, %d(x%d)", rd, imm, rs1);
            } else if(funct3==0x2){
                snprintf(result,buf_size,"lw x%d, %d(x%d)", rd, imm, rs1);
            } else if(funct3==0x4){
                snprintf(result,buf_size,"lbu x%d, %d(x%d)", rd, imm, rs1);
            } else if(funct3==0x5){
                snprintf(result,buf_size,"lhu x%d, %d(x%d)", rd, imm, rs1);
            }
        }
    /* Branch */
        case 0x63: {
            switch (funct3) {
                case 0x0: {
                    snprintf(result, buf_size, "beq x%d, x%d, %d", rs1, rs2, bimm);
                    break;
                }
                case 0x1: {
                    snprintf(result, buf_size, "bne x%d, x%d, %d", rs1, rs2, bimm);
                    break;
                }
                case 0x4: {
                    snprintf(result, buf_size, "blt x%d, x%d, %d", rs1, rs2, bimm);
                    break;
                }
                case 0x5: {
                    snprintf(result, buf_size, "bge x%d, x%d, %d", rs1, rs2, bimm);
                    break;
                }
                case 0x6: {
                    snprintf(result, buf_size, "bltu x%d, x%d, %d", rs1, rs2, bimm);
                    break;
                }
                case 0x7: {
                    snprintf(result, buf_size, "bgeu x%d, x%d, %d", rs1, rs2, bimm);
                    break;
                }
                default: {
                    snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                    break;
                }
            }
            // } else{
            //     snprintf(result, buf_size, "unknown (0x%08x)", instruction);
            // }
            break;
        }   
        case 0x23:{
            if(funct3==0x0){
                snprintf(result,buf_size, "sb x%d ,%d(x%d)",rs2,simm,rs1);
            } else if (funct3==0x1){
                snprintf(result,buf_size, "sh x%d ,%d(x%d)",rs2,simm,rs1);
            }   else if (funct3==0x2){
                snprintf(result,buf_size, "sw x%d ,%d(x%d)",rs2,simm,rs1);
            }
                break;
        }
        case 0x73:{
            switch (funct12){
            {
            case 0x102:{
                snprintf(result,buf_size,"sret");
                break;
            }
            case 0x302:{
                snprintf(result,buf_size,"mret");
                break;
            }
            case 105:{
                snprintf(result,buf_size,"wfi");
                break;
            }
        }
            switch (funct7)
            {
            case 0X11:
                snprintf(result,buf_size,"sfence.vma x%d , x%d", rs1,rs2);
                break;
            }
        }
        }
        default:{
            snprintf(result, buf_size, "unknown (0x%08x)", instruction);
            break;
        }
    /* Jumps */
        case 0x6F: {
            snprintf(result, buf_size, "jal x%d, %d", rd, jimm);
            break;
        }
        case 0x67: {
            snprintf(result, buf_size, "jalr x%d, %d(x%d)", rd, imm, rs1);
            break;
        }
    /* System calls */
        case 0x73: {
            if (funct3 == 0x0) {
                switch (imm) {
                    case 0x0: { // ecall
                        snprintf(result, buf_size, "ecall");
                        break;
                    }
                    default: {
                        snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                        break;
                    }
                }
            } else {
                break;
            }
            break;
        }
    /* Default to unknown if everything fails */
        default: {
            snprintf(result, buf_size, "unknown (0x%08x)", instruction);
            break;
        }
    }
