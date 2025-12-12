    .global _start
_start:
# sub, sll, slt, sltu, xor, srl, sra, or, and,add
addi x1 , x0, 0
    addi x2 , x0 ,0
    addi x3 , x0 ,0
    addi x4 , x0 ,0
    addi x5 , x0 ,0
    addi x6 , x0 ,0
    addi x7 , x0 ,0
    addi x8 , x0 ,0
    addi x9 , x0 ,0
    add x1,x0,5
    add x2,x0,7
    sub x3,x2,x1
    add x4, x0 , 1
    sll x3, x3,x4
    slt x5,x1,x2
    sltu x6 ,x1,x2
    xor x7 , x1,x2
    addi x8 , x0 , 100
    addi x1,x0,1
    srl x1,x8,x1
    sra x2,x8,x1
    or x3,x2,x1
    and x4 ,x2,x1
