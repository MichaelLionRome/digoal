FLAGS :=  -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes \
    -Wold-style-definition -Werror  -Wl,--fatal-warnings

BLOCK_TOPOLOGY=../block-topology/block_topology.o

block.o: block.h block.c ${BLOCK_TOPOLOGY}
	gcc ${FLAGS} -c block.h block.c ${BLOCK_TOPOLOGY}

clean:
	rm -f block.h.gch block.o

all:
	block.o