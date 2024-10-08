
.SUFFIXES: .c .s .o

.c.o:
	$(CC) $(CFLAGS) -c $<

.s.o:
	$(CC) $(CFLAGS) -c $<

all: sagevisor

sagevisor: main.o primitives.o start.o sbi.o vmspaces.o riscv-virt.lds
	$(LD) *.o -o $@ -T riscv-virt.lds ${LDFLAGS}

clean:
	-rm -f *.o sagevisor
