:
qemu-system-riscv64 -smp 3 -nographic -machine virt -m 128M -kernel sophia -bios /usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.elf $@
#qemu-system-riscv64 -smp 3 -nographic -machine virt -m 128M -kernel sophia -bios /usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.elf -S -gdb tcp::10000 $@

