FLAGS :=  -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes \
    -Wold-style-definition -Werror  -Wl,--fatal-warnings

BLOCK_TOPOLOGY_PATH=../block_topology
BLOCK_TOPOLOGY=${BLOCK_TOPOLOGY_PATH}/block_topology.o

block.o: block.h block.c ${BLOCK_TOPOLOGY}
	gcc ${FLAGS} -c block.h block.c ${BLOCK_TOPOLOGY} -I ${BLOCK_TOPOLOGY_PATH}

clean:
	rm -f block.h.gch block.o

all:
	block.o