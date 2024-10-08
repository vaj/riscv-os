:
QEMU=qemu-system-riscv64
OPENSBI=/usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.elf

#$QEMU -smp 2 -nographic -machine virt -m 128M -kernel sageVisor/sagevisor -bios $OPENSBI -device loader,file=guest1 -device loader,file=guest2 -S -gdb tcp::10000 $@
$QEMU -smp 2 -nographic -machine virt -m 128M -kernel sageVisor/sagevisor -bios $OPENSBI -device loader,file=guest1 -device loader,file=guest2 $@

