
PREF=riscv64-unknown-elf-
RVCC = ${PREF}gcc
RVLD = ${PREF}ld
RVAR = ${PREF}ar
RVCFLAGS = -march=rv64g -O2 -mcmodel=medany -ffreestanding -g

.SUFFIXES: .c .s .o

.c.o:
	$(CC) $(CFLAGS) -c $<

all: guestOS1 guestOS2 hyper relocate

relocate:
	cc locateguest.c -o locateguest
	cp sophiaOS/sophia guest1
	cp nowhereOS/nowhere guest2
	./locateguest guest1 0x80600000
	./locateguest guest2 0x84600000

hyper:
	CC=${RVCC} LD=${RVLD} AR=${RVAR} CFLAGS="${RVCFLAGS}" make -C sageVisor

guestOS1:
	CC=${RVCC} LD=${RVLD} AR=${RVAR} CFLAGS="${RVCFLAGS}" make -C sophiaOS

guestOS2:
	CC=${RVCC} LD=${RVLD} AR=${RVAR} CFLAGS="${RVCFLAGS}" make -C nowhereOS

clean:
	-rm -f *.o guest? locateguest
	make clean -C nowhereOS
	make clean -C sophiaOS
	make clean -C sageVisor
