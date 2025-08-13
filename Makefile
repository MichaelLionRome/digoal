CURRENT_PATH=$(shell pwd)
BITVECTOR_INCLUDE=${CURRENT_PATH}/lib/c-bitvector/include
VECTOR_INCLUDE=${CURRENT_PATH}/lib/c-vector/include
CIRCULAR_QUEUE_INCLUDE=${CURRENT_PATH}/lib/c-queue/include
K2TREE_INCLUDES=${CURRENT_PATH}/include

BIN_DEPENDENCIES=-L${CURRENT_PATH}/bin
BIN_LINKS=-lk2tree_merged -lm
BIN=${BIN_DEPENDENCIES} ${BIN_LINKS}

INCLUDES=-I${BITVECTOR_INCLUDE} -I${K2TREE_INCLUDES} -I${VECTOR_INCLUDE} -I${CIRCULAR_QUEUE_INCLUDE}

CFLAGS :=  -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes \
    -Wold-style-definition -Werror -O3

SHARED_CFLAGS := ${CFLAGS} -fPIC

DEBFLAGS := -Wall -Wextra -std=c99 -pedantic -Wmissing-prototypes -Wstrict-prototypes \
    -Wold-style-definition -Werror -g

MAKE_FLAGS=INCLUDES="${INCLUDES}" CFLAGS="${CFLAGS}" BIN="${BIN}"

SHARED_MAKE_FLAGS=INCLUDES="${INCLUDES}" CFLAGS="${SHARED_CFLAGS}" BIN="${BIN}"


DEBUG_FLAGS=INCLUDES="${INCLUDES}" CFLAGS="${DEBFLAGS}" BIN="${BIN}"

MODULES_DIRS := src
RUNNABLE_DIRS := example

COMPR_DIR=k2tree-dyn-compr

build: fetch_deps modules merge-libs merge-libs-noalloc  runnables

all: format build

all-shared: fetch_deps shared-modules merge-libs-shared merge-libs-noalloc-shared

re: clean all

fetch_deps:
	chmod a+x fetch_deps.sh && bash fetch_deps.sh

clean:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) clean -C $$dir ${MAKE_FLAGS};  \
	done

	for dir in ${RUNNABLE_DIRS}; do \
		$(MAKE) clean -C $$dir ${MAKE_FLAGS};  \
	done

	rm -rf _tmp_merge

clean-all: clean
	rm -rf bin lib ${COMPR_DIR} ${COMPR_DIR}.tar.gz
	rm -rf test/build test/.idea test/cmake-build-debug
	

modules:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) -C $$dir ${MAKE_FLAGS}; \
	done

shared-modules:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) -C $$dir ${SHARED_MAKE_FLAGS}; \
	done

runnables:
	for dir in ${RUNNABLE_DIRS}; do \
		$(MAKE) -C $$dir ${MAKE_FLAGS}; \
	done


runnables-debug:
	for dir in ${RUNNABLE_DIRS}; do \
		$(MAKE) -C $$dir ${DEBUG_FLAGS}; \
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

compr: clean-all
	mkdir -p ${COMPR_DIR}
	rsync -rv --exclude=${COMPR_DIR} --exclude=.git --exclude '*.vscode' --exclude '*.idea' . ${COMPR_DIR}
	tar -zcvf ${COMPR_DIR}.tar.gz ${COMPR_DIR}/

merge-libs:
	./mergelibs.sh 1 0

merge-libs-shared:
	./mergelibs.sh 1 1

merge-libs-noalloc:
	./mergelibs.sh 0 0

merge-libs-noalloc-shared:
	./mergelibs.sh 0 1
