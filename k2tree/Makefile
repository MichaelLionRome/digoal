MODULES_DIRS := block-topology block-frontier block morton-code


all: definitions.o modules libk2tree.a
	mkdir -p ../bin && cp libk2tree.a ../bin

re: clean all

clean:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) clean -C $$dir ${MAKE_FLAGS};  \
	done
	rm -rf definitions.o
	rm -rf libk2tree.a

modules:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) -C $$dir ${MAKE_FLAGS}; \
	done

definitions.o: definitions.c
	$(CC) ${CFLAGS} -c definitions.c -o definitions.o ${INCLUDES}


libk2tree.a:
	ar rcs libk2tree.a \
	 block/block.o block/queries_state.o \
	 block-topology/block_topology.o block-frontier/block_frontier.o \
	 morton-code/morton_code.o definitions.o
