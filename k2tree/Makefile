MODULES_DIRS := block-topology block-frontier block morton-code



COMMON_AR= block/block.o block/queries_state.o \
	 block-topology/block_topology.o block-frontier/block_frontier.o \
	 morton-code/morton_code.o definitions.o \
	 custom_bv_handling.o 

all: definitions.o custom_bv_handling.o modules libk2tree.a libk2tree_noalloc.a
	mkdir -p ../bin && cp libk2tree.a ../bin && cp libk2tree_noalloc.a ../bin

re: clean all

clean:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) clean -C $$dir ${MAKE_FLAGS};  \
	done
	rm -rf definitions.o custom_bv_handling.o default_memalloc.o
	rm -rf libk2tree.a libk2tree_noalloc.a


modules:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) -C $$dir ${MAKE_FLAGS}; \
	done

definitions.o: definitions.c
	$(CC) ${CFLAGS} -c definitions.c -o definitions.o ${INCLUDES}

custom_bv_handling.o: custom_bv_handling.c
	$(CC) ${CFLAGS} -c custom_bv_handling.c -o custom_bv_handling.o ${INCLUDES}

default_memalloc.o: default_memalloc.c
	$(CC) ${CFLAGS} -c default_memalloc.c -o default_memalloc.o ${INCLUDES}


libk2tree.a: default_memalloc.o
	ar rcs libk2tree.a ${COMMON_AR} default_memalloc.o

libk2tree_noalloc.a:
	ar rcs libk2tree_noalloc.a ${COMMON_AR}
