BLOCK_TOPOLOGY_PATH=../block-topology
BLOCK_TOPOLOGY=${BLOCK_TOPOLOGY_PATH}/block_topology.o

block.o: ${BLOCK_TOPOLOGY}
	gcc ${CFLAGS} -c block.c -I${BLOCK_TOPOLOGY_PATH} ${INCLUDES}

clean:
	rm -f block.o

all: block.o