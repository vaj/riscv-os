
CC = riscv64-unknown-elf-gcc
LD = riscv64-unknown-elf-ld
CFLAGS = -O2 -mcmodel=medany -ffreestanding -g

.SUFFIXES: .c .s .o

.c.o:
	$(CC) $(CFLAGS) -c $<

.s.o:
	$(CC) $(CFLAGS) -c $<

a.out: main.o primitives.o start.o syscall.o riscv-virt.lds
	$(LD) main.o primitives.o start.o syscall.o -T riscv-virt.lds

clean:
	rm -f *.o a.out
