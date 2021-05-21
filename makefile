
CC = riscv64-unknown-elf-gcc
LD = riscv64-unknown-elf-ld
CFLAGS = -O2 -mcmodel=medany

.SUFFIXES: .c .s .o

.c.o:
	$(CC) $(CFLAGS) -c $<

.s.o:
	$(CC) $(CFLAGS) -c $<

a.out: main.o start.o riscv-virt.lds
	riscv64-unknown-elf-ld main.o start.o -T riscv-virt.lds

clean:
	rm -f *.o a.out
