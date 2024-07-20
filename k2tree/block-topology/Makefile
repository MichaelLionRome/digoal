FLAGS :=  -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes \
    -Wold-style-definition -Werror  -Wl,--fatal-warnings

LIB_PATH=../../lib
BITVECTOR_PATH=${LIB_PATH}/c-bitvector/bitvector
BITVECTOR_O=${BITVECTOR_PATH}/bitvector.o
BITVECTOR_H=${BITVECTOR_PATH}/bitvector.h


block_topology.o: block_topology.h block_topology.c ${BITVECTOR_O}
	gcc ${FLAGS} -c block_topology.h block_topology.c ${BITVECTOR_O} -I ${BITVECTOR_PATH}

clean:
	rm -f block_topology.h.gch block_topology.o

all:
	block_topology.o