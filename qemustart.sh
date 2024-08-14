:
qemu-system-riscv64 -smp 3 -nographic -machine virt -m 128M -kernel sophia -bios none $@
#qemu-system-riscv64 -smp 3 -nographic -machine virt -m 128M -kernel sophia -bios none -S -gdb tcp::10000 $@
