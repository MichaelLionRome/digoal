FLAGS :=  -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes \
    -Wold-style-definition -Werror  -Wl,--fatal-warnings

BLOCK_TOPOLOGY_PATH=../block-topology
BLOCK_TOPOLOGY=${BLOCK_TOPOLOGY_PATH}/block_topology.o

DEPENDENCIES=../deps

block.o: ${BLOCK_TOPOLOGY}
	gcc ${FLAGS} -c block.c -I${BLOCK_TOPOLOGY_PATH} ${INCLUDES}

clean:
	rm -f block.o

all: block.o