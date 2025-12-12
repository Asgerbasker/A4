    .global _start
_start:

    addi x1, x0, 1
    addi x2, x0, 2
    addi x3, x0, 300
    addi x4, x0, 400
    addi x5, x0, 500
    addi x6, x0, 600
    addi x7, x0, 7
    addi x8, x0, 800
    addi x9, x0, 900
    mul x1, x1, x2
    mulh x2, x3, x4
    mulhsu x1, x8, x9
    mulhu x1, x5, x6
    div x1, x4, x4
    divu x1, x4, x4
    rem x1, x6, x7
    remu x1, x6, x7
    addi x17, x0, 3
    ecall
    