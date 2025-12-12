    .global _start
_start:
    # Initialize registers
    addi x1, x0, 0      # x1 = 0
    addi x2, x0, 8      # x2 = 8
    addi x3, x0, 0      # x3 = 0, will store result

    # Use JAL to jump forward 16 bytes (4 instructions)
    jal x4, label1      # x4 = return address, jump to label1

    addi x3, x3, 1      # skipped because of JAL
    addi x3, x3, 2      # skipped

label1:
    addi x3, x3, 10     # x3 = x3 + 10
    addi x5, x0, 100    # x5 = 100, will use with JALR

    # Use JALR to jump indirectly
    # x2 points to memory
    li   x3, 0x00000513   # addi x10, x0, 0   (encoded instruction)
    sw   x3, 0(x2)        # write instruction into memory
    jalr x0, 0(x2)        # jump and execute the instruction

    # Instructions after JALR (not executed if jump works)
    addi x3, x3, 1
    addi x3, x3, 1

    # Target of JALR
jalr_target:
    addi x3, x3, 50     # modify x3 to show we landed here
    addi x7, x0, 0      # just a dummy instruction

    # Program end (in a real simulator you might exit)
    nop
