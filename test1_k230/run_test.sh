#!/bin/bash
#./../build/qemu-system-riscv64 -M k230 -nographic -bios kernel.elf
./../build/qemu-system-riscv64 -M k230 -nographic -bios kernel.elf -d unimp,mmu,int

