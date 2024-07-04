:
qemu-system-riscv64 -smp 3 -nographic -machine virt -m 128M -kernel a.out -bios none $@
#qemu-system-riscv64 -smp 3 -nographic -machine virt -m 128M -kernel a.out -bios none -S -gdb tcp::10000 $@
