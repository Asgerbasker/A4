#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "disassemble.h"
#include "helpers.h"

void disassemble(uint32_t addr, uint32_t instruction, char *result, size_t buf_size, struct symbols *symbols) {
    uint32_t op_code = get_opcode(instruction);
    uint32_t funct3 = get_funct3(instruction);
    uint32_t funct7 = get_funct7(instruction);
    uint32_t rd = get_rd(instruction);
    uint32_t rs1 = get_rs1(instruction);
    uint32_t rs2 = get_rs2(instruction);
    int32_t imm = get_imm(instruction); 
    int32_t uimm = get_upper_imm(instruction);
    int32_t simm = get_store_imm(instruction);
    int32_t bimm = get_branch_imm(instruction);
    int32_t jimm = get_jump_imm(instruction);
    uint32_t shamt = get_rs2(instruction);
    uint32_t branch_target = addr + bimm;
    uint32_t jump_target = addr + jimm;

    switch (op_code) {
        /* I-type ALU (immediate) */
        case 0x13: {
            switch (funct3) {
                case 0x0: {
                    snprintf(result, buf_size, "addi x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x1: {
                    if (funct7 == 0x0) {
                        snprintf(result, buf_size, "slli x%d, x%d, %d", rd, rs1, shamt);
                    } 
                    else {
                        snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                        break;
                    }
                    break;
                }
                case 0x2: {
                    snprintf(result, buf_size, "slti x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x3: {
                    snprintf(result, buf_size, "sltiu x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x4: {
                    snprintf(result, buf_size, "xori x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x6: {
                    snprintf(result, buf_size, "ori x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                case 0x5: {
                    if (funct7 == 0x0) { 
                        snprintf(result, buf_size, "srli x%d, x%d, %d", rd, rs1, shamt);
                    } 
                    else if (funct7 == 0x20) {
                        snprintf(result, buf_size, "srai x%d, x%d, %d", rd, rs1, shamt);
                    } 
                    else {
                        snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                        break;
                    }
                    break;
                }
                case 0x7: {
                    snprintf(result, buf_size, "andi x%d, x%d, %d", rd, rs1, imm);
                    break;
                }
                default: {
                    snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                    break;
                }
                break;
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
                case 0x0: { 
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
                        break;
                    }
                    break;
                }
                case 0x01: { 
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
                        break;
                    }
                    break;
                }
                case 0x20: { 
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
                        break;
                    }
                    break;
                }
                default: {
                    snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                    break;
                }
                break;
            }
            break;
        }
        /* Load */
        case 0x3: {
            if (funct3 == 0x0) {
                snprintf(result, buf_size, "lb x%d, %d(x%d)", rd, imm, rs1);
                break;
            } 
            else if (funct3 == 0x1) {
                snprintf(result, buf_size, "lh x%d, %d(x%d)", rd, imm, rs1);
                break;
            } 
            else if (funct3 == 0x2) {
                snprintf(result, buf_size, "lw x%d, %d(x%d)", rd, imm, rs1);
                break;
            } 
            else if (funct3 == 0x4) {
                snprintf(result, buf_size, "lbu x%d, %d(x%d)", rd, imm, rs1);
                break;
            } 
            else if (funct3 == 0x5) {
                snprintf(result, buf_size, "lhu x%d, %d(x%d)", rd, imm, rs1);
                break;
            } 
            else {
                snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                break;
            }
            break;
        }
        /* Branch */
        case 0x63: {
            switch (funct3) {
                case 0x0: {
                    snprintf(result, buf_size, "beq x%d, x%d, %x", rs1, rs2, branch_target);
                    break;
                }
                case 0x1: {
                    snprintf(result, buf_size, "bne x%d, x%d, %x", rs1, rs2, branch_target);
                    break;
                }
                case 0x4: {
                    snprintf(result, buf_size, "blt x%d, x%d, %x", rs1, rs2, branch_target);
                    break;
                }
                case 0x5: {
                    snprintf(result, buf_size, "bge x%d, x%d, %x", rs1, rs2, branch_target);
                    break;
                }
                case 0x6: {
                    snprintf(result, buf_size, "bltu x%d, x%d, %x", rs1, rs2, branch_target);
                    break;
                }
                case 0x7: {
                    snprintf(result, buf_size, "bgeu x%d, x%d, %x", rs1, rs2, branch_target);
                    break;
                }
                default: {
                    snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                    break;
                }
                break;
            }
            break;
        }   
        /* Store */    
        case 0x23: {
            if (funct3 == 0x0) {
                snprintf(result, buf_size, "sb x%d, %d(x%d)", rs2, simm, rs1);
                break;
            } 
            else if (funct3 == 0x1) {
                snprintf(result, buf_size, "sh x%d, %d(x%d)", rs2, simm, rs1);
                break;
            } 
            else if (funct3 == 0x2) {
                snprintf(result, buf_size, "sw x%d, %d(x%d)", rs2, simm, rs1);
                break;
            } 
            else {
                snprintf(result, buf_size, "unknown (0x%08x)", instruction);
                break;
            }
            break;
        }
        /* Jumps */
        case 0x6F: {
            snprintf(result, buf_size, "jal x%d, %x", rd, jump_target);
            break;
        }
        case 0x67: {
            snprintf(result, buf_size, "jalr x%d, %d(x%d)", rd, imm, rs1);
            break;
        }
        /* System calls */
        case 0x73: {
            if (funct3 == 0 && imm == 0x0) {
                snprintf(result, buf_size, "ecall");
            } 
            else if (funct3 == 0 && funct7 == 0x09) {
                snprintf(result, buf_size, "sfence.vma x%d, x%d", rs1, rs2);
            } 
            else if (imm == 0x102) {
                snprintf(result, buf_size, "sret");
            } 
            else if (imm == 0x302) {
                snprintf(result, buf_size, "mret");
            } 
            else if (imm == 0x105) {
                snprintf(result, buf_size, "wfi");
            } 
            else {
                snprintf(result, buf_size, "unknown (0x%08x)", instruction);
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
}