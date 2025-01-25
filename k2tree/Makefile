MODULES_DIRS := block-topology block-frontier block

all: definitions.o modules

re: clean all

clean:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) clean -C $$dir ${MAKE_FLAGS};  \
	done
	rm -rf definitions.o

modules:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) -C $$dir ${MAKE_FLAGS}; \
	done

definitions.o: definitions.c
	$(CC) ${CFLAGS} -c definitions.c -o definitions.o ${INCLUDES}
