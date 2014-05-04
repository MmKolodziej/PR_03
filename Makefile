CC=mpicc
C_FLAGS=-Wall  -g
L_FLAGS=-lm -lrt
HOSTFILE=hosts
PROGRAM=zad3

ifeq ($(strip $(plik)),)
  plik := ./mapa
endif

SOURCES=$(wildcard *.c)
BINARIES=$(SOURCES:%.c=%.o)

all: $(PROGRAM)

$(PROGRAM): $(BINARIES)
	$(CC) -o $@ $^ $(C_FLAGS)

%.c.o: %.c
	$(CC) -c -o $@ $< $(C_FLAGS)

.PHONY: clean run

compile: all

run: all
	mpirun -n $(n) --hostfile ${HOSTFILE} ./${PROGRAM} $(plik)

debug: 
	mpirun -n $(n) --debug --hostfile ${HOSTFILE} ./${PROGRAM} $(plik)

clean:
	-rm -f *.o $(PROGRAM)
 
