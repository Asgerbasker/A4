    .global _start
_start:
#addi, 1slli, 1slti, 1sltiu, 1xori, 1srli, srai, 1ori, 1andi.
#lui, auipc
#1.1 load load upper imm
    addi x1, x0, 0
    addi x2, x0, 0
    addi x3, x0, 0
    addi x4, x0, 0
    addi x5, x0, 0
    addi x6, x0, 0
    addi x7, x0, 0
    addi x8, x0, 0
    addi x9, x0, 0
#1.3 addi
    addi x1, x0, 4
    addi x2, x2, 3
#1.4 set less than immediate
    slti x3, x2, 4 # x8 should be 1
#1.5 sltiu
    sltiu x4, x1, 2 # x9 should be 0
#1.9 slli
    addi x5, x0, 1 #x5=1
    slli x5, x5, 2 #x5=4
#1.10 srli 
    addi x6, x0, 15 #x6=15
    srli x6, x6, 1 # x6=7
#1.6 xori
    addi x7, x0, 5 
    xori x7, x7, 4
#1.7 ori
    addi x8, x0, 5 
    ori x8, x8, 5
#1.11 srai
    addi x9, x0, 1
    srai x9, x9, 2
    addi x17, x0, 3
    ecall 
