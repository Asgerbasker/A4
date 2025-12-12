    .global _start
_start:
  #sb, sh, sw  lb, lh, lw, lbu, lhu
 
    addi x1, x0, 0
    addi x2, x0, 0
    addi x3, x0, 0
    addi x4, x0, 0
    addi x5, x0, 0
    addi x6, x0, 0
    addi x7, x0, 0
    addi x8, x0, 0
    addi x9, x0, 0

    addi x1, x0, 6
    sb x1, 1(x2)
    lb x1, 1(x2)
    lbu x3, 1(x2)

    addi x4, x0, 10
    sh x4, 2(x5)
    lh x4, 2(x5)
    lhu x8, 2(x5)

    addi x6, x0, 168
    sw x6, 8(x7)
    lw x6, 8(x7)

    addi x17, x0, 3
    ecall





