
CFLAGS += -msmall-data-limit=0

.SUFFIXES: .c .s .o

.c.o:
	$(CC) $(CFLAGS) -c $<

.s.o:
	$(CC) $(CFLAGS) -c $<

all: nowhere

nowhere: main.o start.o sbi.o riscv-virt.lds
	$(LD) *.o -o $@ -T riscv-virt.lds ${LDFLAGS}

clean:
	-rm -f *.o nowhere
