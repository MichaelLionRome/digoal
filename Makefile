CURRENT_PATH=$(shell pwd)
BITVECTOR_INCLUDE=${CURRENT_PATH}/lib/c-bitvector/include
VECTOR_INCLUDE=${CURRENT_PATH}/lib/c-vector/include
CIRCULAR_QUEUE_INCLUDE=${CURRENT_PATH}/lib/c-queue/include
K2TREE_INCLUDES=${CURRENT_PATH}/k2tree

BIN_DEPENDENCIES=-L${CURRENT_PATH}/bin -L${CURRENT_PATH}/lib/c-bitvector/bin -L${CURRENT_PATH}/lib/c-queue/bin -L${CURRENT_PATH}/lib/c-vector/bin 
BIN_LINKS=-lk2tree -lbitvector -lcircular_queue -lvector -lm
BIN=${BIN_DEPENDENCIES} ${BIN_LINKS}

INCLUDES=-I${BITVECTOR_INCLUDE} -I${K2TREE_INCLUDES} -I${VECTOR_INCLUDE} -I${CIRCULAR_QUEUE_INCLUDE}

CFLAGS :=  -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes \
    -Wold-style-definition -Werror -O3

DEBFLAGS := -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes \
    -Wold-style-definition -Werror -g

MAKE_FLAGS=INCLUDES="${INCLUDES}" CFLAGS="${CFLAGS}" BIN="${BIN}"

DEBUG_FLAGS=INCLUDES="${INCLUDES}" CFLAGS="${DEBFLAGS}" BIN="${BIN}"

MODULES_DIRS := k2tree example



all: fetch_deps format modules

re: clean all

fetch_deps:
	chmod a+x fetch_deps.sh && bash fetch_deps.sh

clean:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) clean -C $$dir ${MAKE_FLAGS};  \
	done

clean-all: clean
	rm -rf bin lib

modules:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) -C $$dir ${MAKE_FLAGS}; \
	done

debug:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) -C $$dir ${DEBUG_FLAGS}; \
	done

format:
	find . -path ./lib -prune -o -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;

test-build:
	cd test && mkdir -p build && cd build && cmake .. && make

test-all: test-build
	./test/build/block_test

memcheck:
	valgrind --leak-check=full ./test/build/block_leak_test