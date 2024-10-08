
CFLAGS_APP = ${CFLAGS} -msmall-data-limit=0

.SUFFIXES: .c .s .o

.c.o:
	$(CC) $(CFLAGS) -c $<

.s.o:
	$(CC) $(CFLAGS) -c $<

all: sophia

sophia: main.o vma.o primitives.o start.o syscall.o sbi.o application.o riscv-virt.lds
	$(LD) main.o vma.o primitives.o sbi.o start.o syscall.o application.o -T riscv-virt.lds -o $@ ${LDFLAGS}

application.o: application.c
	$(CC) $(CFLAGS_APP) -c $<

clean:
	-rm -f sophia *.o
