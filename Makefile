CURRENT_PATH=$(shell pwd)
BITVECTOR_INCLUDE=${CURRENT_PATH}/lib/c-bitvector/include
VECTOR_INCLUDE=${CURRENT_PATH}/lib/c-vector/include
CIRCULAR_QUEUE_INCLUDE=${CURRENT_PATH}/lib/c-queue/include
K2TREE_INCLUDES=${CURRENT_PATH}/k2tree

#BIN_DEPENDENCIES=-L${CURRENT_PATH}/bin -L${CURRENT_PATH}/lib/c-bitvector/bin -L${CURRENT_PATH}/lib/c-queue/bin -L${CURRENT_PATH}/lib/c-vector/bin 
#BIN_LINKS=-lk2tree -lbitvector -lcircular_queue -lvector -lm
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

MODULES_DIRS := k2tree
RUNNABLE_DIRS := example

COMPR_DIR=k2tree-dyn-compr

all: fetch_deps format modules merge-libs runnables

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

clean-all: clean
	rm -rf bin lib ${COMPR_DIR} ${COMPR_DIR}.tar.gz
	rm -rf test/build test/.idea test/cmake-build-debug

modules:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) -C $$dir ${MAKE_FLAGS}; \
	done

shared_modules:
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
	mkdir -p _tmp_merge
	# put all libs into _tmp_merge
	cp bin/libk2tree.a _tmp_merge/
	cp ${CURRENT_PATH}/lib/c-bitvector/bin/libbitvector.a _tmp_merge/
	cp ${CURRENT_PATH}/lib/c-vector/bin/libvector.a _tmp_merge/
	cp ${CURRENT_PATH}/lib/c-queue/bin/libcircular_queue.a _tmp_merge/
	cd _tmp_merge && \
	ar -x libk2tree.a && \
	ar -x libbitvector.a && \
	ar -x libvector.a && \
	ar -x libcircular_queue.a && \
	ar -qc libk2tree_merged.a *.o && \
	cp libk2tree_merged.a ../bin/
	rm -rf _tmp_merge
