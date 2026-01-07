all:
	riscv64-unknown-elf-as -o kernel.o kernel.s
	riscv64-unknown-elf-ld -T link.ld -o kernel.elf kernel.o
	riscv64-unknown-elf-objcopy -O binary kernel.elf kernel.bin


