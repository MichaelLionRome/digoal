
COMMON_AR= block.o \
		queries_state.o \
		block_topology.o \
		block_frontier.o \
	 	morton_code.o \
		definitions.o \
	 	custom_bv_handling.o 

all: all-objects libs-copy

re: clean all

clean: clean-objects clean-libs


# Object files

all-objects: block.o queries_state.o block_frontier.o block_topology.o \
			morton_code.o definitions.o custom_bv_handling.o default_memalloc.o

block.o: block.c
	$(CC) ${CFLAGS} -c block.c -o block.o ${INCLUDES}

queries_state.o: queries_state.c
	$(CC) ${CFLAGS} -c queries_state.c -o queries_state.o ${INCLUDES}

block_frontier.o: block_frontier.c
	$(CC) ${CFLAGS} -c block_frontier.c -o block_frontier.o ${INCLUDES}

block_topology.o: block_topology.c
	$(CC) ${CFLAGS} -c block_topology.c -o block_topology.o ${INCLUDES}
	
morton_code.o: morton_code.c
	$(CC) ${CFLAGS} -c morton_code.c -o morton_code.o ${INCLUDES}

definitions.o: definitions.c
	$(CC) ${CFLAGS} -c definitions.c -o definitions.o ${INCLUDES}

custom_bv_handling.o: custom_bv_handling.c
	$(CC) ${CFLAGS} -c custom_bv_handling.c -o custom_bv_handling.o ${INCLUDES}

default_memalloc.o: default_memalloc.c
	$(CC) ${CFLAGS} -c default_memalloc.c -o default_memalloc.o ${INCLUDES}


clean-objects:
	rm -f \
		block.o \
		queries_state.o \
		block_frontier.o \
		block_topology.o \
		morton_code.o \
		definitions.o \
		custom_bv_handling.o \
		default_memalloc.o

# Lib files

libs-copy: libk2tree.a libk2tree_noalloc.a
	mkdir -p ../bin && \
	cp libk2tree.a ../bin && \
	cp libk2tree_noalloc.a ../bin

libk2tree.a: default_memalloc.o
	ar rcs libk2tree.a ${COMMON_AR} default_memalloc.o

libk2tree_noalloc.a:
	ar rcs libk2tree_noalloc.a ${COMMON_AR}

clean-libs:
	rm -rf libk2tree.a libk2tree_noalloc.a
