CC=mpiCC
C_FLAGS=-Wall -g
L_FLAGS=-lm -lrt
TARGET=main
FILES=main.o
SOURCE=main.cpp
HOSTFILE=hosts

${TARGET}: ${FILES}
	${CC} -o ${TARGET} ${FILES} ${L_FLAGS}

${FILES}: ${SOURCE}
	${CC} -o ${FILES} -c ${SOURCE} ${C_FLAGS}

.PHONY: clean run

run:
	mpirun --hostfile ${HOSTFILE} ./${TARGET} $(n) $(k) $(mapa)

clean:
	-rm -f ${FILES} ${TARGET}