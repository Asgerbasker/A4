    .global _start
_start:
#beq, bne, blt, bge, bltu, bgeu
    addi x1 , x0, 0
    addi x2 , x0 ,0
    addi x3 , x0 ,0
    addi x4 , x0 ,0
    addi x5 , x0 ,0
    beq x5,x1, L1
    addi x1,x0,8
L1:
    addi x5,x0,10
    bne x5, x1, L2
    addi x1,x0,5
L2:
    addi x1,x0,5
    blt x1 ,x5, L3
    addi x5,x5,5
L3:
    bge x5,x1,L4
    addi x5,x5,5
L4:
    bltu x5,x1, L5
    addi x5,x5,5
L5:
    bgeu x5,x1 ,L6
    addi x5,x5,5
L6:
    addi x5,x5,5

