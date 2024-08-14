
CC = riscv64-unknown-elf-gcc
LD = riscv64-unknown-elf-ld
CFLAGS = -march=rv64g -O2 -mcmodel=medany -ffreestanding -g
CFLAGS_APP = -march=rv64g -O2 -mcmodel=medany -ffreestanding -g -msmall-data-limit=0

.SUFFIXES: .c .s .o

.c.o:
	$(CC) $(CFLAGS) -c $<

.s.o:
	$(CC) $(CFLAGS) -c $<

sophia: main.o primitives.o start.o syscall.o application.o riscv-virt.lds
	$(LD) main.o primitives.o start.o syscall.o application.o -T riscv-virt.lds -o $@

application.o: application.c
	$(CC) $(CFLAGS_APP) -c $<

clean:
	-rm -f sophia *.o
