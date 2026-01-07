.section .text
.global _start

_start:
    li t0, 0x91400004       # UART TX register
    li a0, 'H'              # character
    sb a0, 0(t0)            # write directly
    j _start                      # infinite loop

