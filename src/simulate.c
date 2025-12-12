#include "simulate.h"
#include "disassemble.h"
#include "helpers.h"

Stat simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols) {
    Stat stat = {0};
    const unsigned sizes[4] = {256, 1024, 4096, 16384};

    for (int i = 0; i < 4; i++) {
        bimodal_init(&stat.bimodal[i], sizes[i]);
        gshare_init(&stat.gshare[i], sizes[i]);
    }

    // 32 registers
    int32_t regs[32];

    // zero registers
    for (int i = 0; i < 32; i++) {
        regs[i] = 0;
    }

    // program counter 
    uint32_t pc = (uint32_t)start_addr; 
    int running = 1;
    // if we jump to next instruction
    int mark_target = 0;

    while (running) {
        // fetch instruction
        uint32_t instruction = (uint32_t)memory_rd_w(mem, pc);

        // informations about sideeffects for the log
        int branch_logged = 0;
        int branch_taken = 0;
        int reg_written = -1;
        uint32_t reg_value = 0;
        int mem_written = 0;
        uint32_t mem_addr = 0;
        uint32_t mem_val = 0;
        int mem_size = 0; // 1, 2 or 4

        // disassemble to log file
        if (log_file) {
            char buf[128];
            disassemble(pc, instruction, buf, sizeof(buf), symbols);
            fprintf(log_file, "%6ld %s %05x : %08x  %s", stat.insns + 1, mark_target ? "=>" : "  ", pc, instruction, buf);
            mark_target = 0;
        }
        stat.insns++;

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

        int32_t v_rs1 = regs[rs1];
        int32_t v_rs2 = regs[rs2];

        uint32_t next_pc = pc + 4;

        switch (op_code) {
            case 0x37: { // lui
                write_reg(rd, uimm, regs);
                reg_written = rd;
                reg_value = uimm;
                break;
            }
            case 0x17: { // auipc
                write_reg(rd, pc + uimm, regs);
                reg_written = rd;
                reg_value = pc + uimm;
                break;
            }
            case 0x6F: { // jal
                write_reg(rd, pc + 4, regs);
                next_pc = pc + jimm;
                mark_target = 1; 
                reg_written = rd;
                reg_value = pc + 4;
                break;
            }
            case 0x67: { // jalr
                if (funct3 == 0x0) {
                    uint32_t target = (uint32_t)(v_rs1 + imm);
                    target &= ~1u; // LSB = 0
                    write_reg(rd, pc + 4, regs);
                    next_pc = target;
                    mark_target = 1;
                    reg_written = rd;
                    reg_value = pc + 4;
                }
                break;
            }
            /* Branches */
            case 0x63: {
                switch (funct3) {
                    case 0x0: { // beq
                        branch_taken = (v_rs1 == v_rs2);
                        break;
                    }
                    case 0x1: { // bne
                        branch_taken = (v_rs1 != v_rs2);
                        break;
                    }
                    case 0x4: { // blt
                        branch_taken = (v_rs1 < v_rs2);
                        break;
                    }
                    case 0x5: { // bge
                        branch_taken = (v_rs1 >= v_rs2);
                        break;
                    }
                    case 0x6: { // bltu
                        branch_taken = ((uint32_t)v_rs1 < (uint32_t)v_rs2);
                        break;
                    }
                    case 0x7: { // bgeu
                        branch_taken = ((uint32_t)v_rs1 >= (uint32_t)v_rs2);
                        break;
                    }
                    default: {
                        break;
                    }
                }
                if (branch_taken) {
                    next_pc = pc + bimm;
                    mark_target = 1;
                }
                // log info about jump
                branch_logged = 1;

                // update jump predictors
                // NT + BTFNT
                pred_nt(&stat.nt, branch_taken);
                pred_btfnt(&stat.btfnt, branch_taken, bimm);
                // Bimodal + gShare
                for (int i = 0; i < 4; i++) {
                    bimodal_update(&stat.bimodal[i], pc, branch_taken);
                    gshare_update(&stat.gshare[i], pc, branch_taken);
                }
                break;
            }
            /* I-type ALU  */
            case 0x13: {
                switch (funct3) {
                    case 0x0: { // addi
                        write_reg(rd, v_rs1 + imm, regs);
                        reg_written = rd;
                        reg_value = v_rs1 + imm;
                        break;
                    }
                    case 0x2: { // slti
                        write_reg(rd, (v_rs1 < imm) ? 1 : 0, regs);
                        reg_written = rd;
                        reg_value = (v_rs1 < imm) ? 1 : 0;
                        break;
                    }
                    case 0x3: { // sltiu
                        write_reg(rd, ((uint32_t)v_rs1 < (uint32_t)imm) ? 1 : 0, regs);
                        reg_written = rd;
                        reg_value = ((uint32_t)v_rs1 < (uint32_t)imm) ? 1 : 0;
                        break;
                    }
                    case 0x4: { // xori
                        write_reg(rd, v_rs1 ^ imm, regs);
                        reg_written = rd;
                        reg_value = v_rs1 ^ imm;
                        break;
                    }
                    case 0x6: { // ori
                        write_reg(rd, v_rs1 | imm, regs);
                        reg_written = rd;
                        reg_value = v_rs1 | imm;
                        break;
                    }
                    case 0x7: { // andi
                        write_reg(rd, v_rs1 & imm, regs);
                        reg_written = rd;
                        reg_value = v_rs1 & imm;
                        break;
                    } 
                    case 0x1: { // slli
                        if (funct7 == 0x0) {
                            write_reg(rd, (int32_t)((uint32_t)v_rs1 << shamt), regs);
                            reg_written = rd;
                            reg_value = (int32_t)((uint32_t)v_rs1 << shamt);
                        }
                        break;
                    }
                    case 0x5: { // srli / srai
                        if (funct7 == 0x00) { // srli (logic)
                            write_reg(rd, (int32_t)((uint32_t)v_rs1 >> shamt), regs);
                            reg_written = rd;
                            reg_value = (int32_t)((uint32_t)v_rs1 >> shamt);
                        } 
                        else if (funct7 == 0x20) { // srai (arithmetic)
                            write_reg(rd, v_rs1 >> shamt, regs);
                            reg_written = rd;
                            reg_value = v_rs1 >> shamt;
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            /* R-type ALU including M-extension */
            case 0x33: {
                if (funct7 == 0x0) {
                    switch (funct3) {
                        case 0x0: { // add
                            write_reg(rd, v_rs1 + v_rs2, regs);
                            reg_written = rd;
                            reg_value = v_rs1 + v_rs2;
                            break;
                        }
                        case 0x1: { // sll
                            write_reg(rd, (int32_t)((uint32_t)v_rs1 << (v_rs2 & 0x1F)), regs);
                            reg_written = rd;
                            reg_value = (int32_t)((uint32_t)v_rs1 << (v_rs2 & 0x1F));
                            break;
                        }
                        case 0x2: { // slt
                            write_reg(rd, (v_rs1 < v_rs2) ? 1 : 0, regs);
                            reg_written = rd;
                            reg_value = (v_rs1 < v_rs2) ? 1 : 0;
                            break;
                        }
                        case 0x3: { // sltu
                            write_reg(rd, ((uint32_t)v_rs1 < (uint32_t)v_rs2) ? 1 : 0, regs);
                            reg_written = rd;
                            reg_value = ((uint32_t)v_rs1 < (uint32_t)v_rs2) ? 1 : 0;
                            break;
                        }
                        case 0x4: { // xor
                            write_reg(rd, v_rs1 ^ v_rs2, regs);
                            reg_written = rd;
                            reg_value = v_rs1 ^ v_rs2;
                            break;
                        }
                        case 0x5: { // srl
                            write_reg(rd, (int32_t)((uint32_t)v_rs1 >> (v_rs2 & 0x1F)), regs);
                            reg_written = rd;
                            reg_value = (int32_t)((uint32_t)v_rs1 >> (v_rs2 & 0x1F));
                            break;
                        }
                        case 0x6: { // or
                            write_reg(rd, v_rs1 | v_rs2, regs);
                            reg_written = rd;
                            reg_value = v_rs1 | v_rs2;
                            break;
                        }
                        case 0x7: { // and
                            write_reg(rd, v_rs1 & v_rs2, regs);
                            reg_written = rd;
                            reg_value = v_rs1 & v_rs2;
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
                else if (funct7 == 0x20) {
                    switch (funct3) {
                        case 0x0: { // sub
                            write_reg(rd, v_rs1 - v_rs2, regs);
                            reg_written = rd;
                            reg_value = v_rs1 - v_rs2;
                            break;
                        }
                        case 0x5: { // sra
                            write_reg(rd, v_rs1 >> (v_rs2 & 0x1F), regs);
                            reg_written = rd;
                            reg_value = v_rs1 >> (v_rs2 & 0x1F);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
                else if (funct7 == 0x01) {
                    switch (funct3) {
                        case 0x0: { // mul
                            write_reg(rd, rv32m_mul(v_rs1, v_rs2), regs);
                            reg_written = rd;
                            reg_value = rv32m_mul(v_rs1, v_rs2);
                            break;
                        }
                        case 0x1: { // mulh
                            write_reg(rd, rv32m_mulh(v_rs1, v_rs2), regs);
                            reg_written = rd;
                            reg_value = rv32m_mulh(v_rs1, v_rs2);
                            break;
                        }
                        case 0x2: { // mulhsu
                            write_reg(rd, rv32m_mulhsu(v_rs1, (uint32_t)v_rs2), regs);
                            reg_written = rd;
                            reg_value = rv32m_mulhsu(v_rs1, (uint32_t)v_rs2);
                            break;
                        }
                        case 0x3: { // mulhu
                            write_reg(rd, rv32m_mulhu((uint32_t)v_rs1, (uint32_t)v_rs2), regs);
                            reg_written = rd;
                            reg_value = rv32m_mulhu((uint32_t)v_rs1, (uint32_t)v_rs2);
                            break;
                        }
                        case 0x4: { // div
                            write_reg(rd, rv32m_div(v_rs1, v_rs2), regs);
                            reg_written = rd;
                            reg_value = rv32m_div(v_rs1, v_rs2);
                            break;
                        }
                        case 0x5: { // divu
                            write_reg(rd, (int32_t)rv32m_divu((uint32_t)v_rs1, (uint32_t)v_rs2), regs);
                            reg_written = rd;
                            reg_value = (int32_t)rv32m_divu((uint32_t)v_rs1, (uint32_t)v_rs2);
                            break;
                        }
                        case 0x6: { // rem
                            write_reg(rd, rv32m_rem(v_rs1, v_rs2), regs);
                            reg_written = rd;
                            reg_value = rv32m_rem(v_rs1, v_rs2);
                            break;
                        }
                        case 0x7: { // remu
                            write_reg(rd, (int32_t)rv32m_remu((uint32_t)v_rs1, (uint32_t)v_rs2), regs);
                            reg_written = rd;
                            reg_value = (int32_t)rv32m_remu((uint32_t)v_rs1, (uint32_t)v_rs2);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
                break;
            }
            /* Stores (S-Type) */
            case 0x23: {
                uint32_t addr = (uint32_t)(v_rs1 + simm);
                switch (funct3) {
                    case 0x0: { // sb 
                        memory_wr_b(mem, addr, v_rs2);
                        mem_written = 1;
                        mem_addr = addr;
                        mem_val = (uint8_t)v_rs2;
                        mem_size = 1;
                        break;
                    }
                    case 0x1: { // sh
                        memory_wr_h(mem, addr, v_rs2);
                        mem_written = 1;
                        mem_addr = addr;
                        mem_val = (uint16_t)v_rs2;
                        mem_size = 2;
                        break;
                    }
                    case 0x2: { // sw
                        memory_wr_w(mem, addr, v_rs2);
                        mem_written = 1;
                        mem_addr = addr;
                        mem_val = (uint32_t)v_rs2;
                        mem_size = 4;
                        break;
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            /* Loads (I-type) */
            case 0x03: {
                uint32_t addr = (uint32_t)(v_rs1 + imm);
                int32_t val = 0;
                switch (funct3) {
                    case 0x0: { // lb
                        int32_t byte = memory_rd_b(mem, addr);
                        val = (byte << 24) >> 24; // sign-extend byte
                        break;
                    }
                    case 0x1: { // lh
                        int32_t half = memory_rd_h(mem, addr);
                        val = (half << 16) >> 16; // sign-extend halfword
                        break;
                    }
                    case 0x2: { // lw
                        val = memory_rd_w(mem, addr);
                        break;
                    }
                    case 0x4: { // lbu
                        uint32_t byte = (uint32_t)memory_rd_b(mem, addr);
                        val = (int32_t)(byte & 0xFF);
                        break;
                    }
                    case 0x5: { // lhu
                        uint32_t half = (uint32_t)memory_rd_h(mem, addr);
                        val = (int32_t)(half & 0xFFFF);
                        break;
                    }
                    default: {
                        break;
                    }
                }
                write_reg(rd, val, regs);
                reg_written = rd;
                reg_value = val;
                break;
            }
            /* System calls */
            case 0x73: {
                if (funct3 == 0x0 && imm == 0x000) {
                    if (!handle_syscall(regs)) {
                        running = 0;
                    }
                }
                break;
            }
            default: {
                fprintf(stderr, "Unknown/not implemented instruction. @%08x: %08x\n", pc, instruction);
                running = 0;
                break;
            }
        }
        if (log_file) {
            // conditional jump: {T} / {F}
            if (branch_logged) {
                fprintf(log_file, "    {%c}", branch_taken ? 'T' : 'F');
            }
            // Register writes: R[..] <- ..
            if (reg_written > 0 && reg_written < 32) { // x0 ignoreres
                fprintf(log_file, "    R[%2d] <- %x", reg_written, reg_value);
            }
            // Memory write: M[..] <- ..
            if (mem_written) {
                switch (mem_size) {
                    case 1:
                        fprintf(log_file, "    M[%08x] <- %02x", mem_addr, (unsigned)(mem_val & 0xFF));
                        break;
                    case 2:
                        fprintf(log_file, "    M[%08x] <- %04x", mem_addr, (unsigned)(mem_val & 0xFFFF));
                        break;
                    case 4:
                        fprintf(log_file, "    M[%08x] <- %08x", mem_addr, (unsigned)mem_val);
                        break;
                }
            }
            fputc('\n', log_file);
        }
        regs[0] = 0;
        pc = next_pc;
    }
    return stat;
}