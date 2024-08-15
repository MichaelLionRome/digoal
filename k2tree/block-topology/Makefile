FLAGS :=  -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes \
    -Wold-style-definition -Werror  -Wl,--fatal-warnings

DEPENDENCIES=../deps

block_topology.o:
	gcc ${FLAGS} -c block_topology.c ${INCLUDES}
	
clean:
	rm -f block_topology.o

all: block_topology.o