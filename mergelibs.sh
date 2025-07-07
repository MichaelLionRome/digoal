#!/bin/bash

CURRENT_PATH="$(pwd)"

if [ "$1" -eq "1" ]; then # ALLOC
	SELECTED_LIB=libk2tree.a
	ALLOC_SUFFIX=
else
	SELECTED_LIB=libk2tree_noalloc.a
	ALLOC_SUFFIX=_noalloc
fi

if [ "$2" -eq "1" ]; then # SHARED
	SELECTED_BVLIB=libbitvector_se.a
	SHARED_SUFFIX=_shared
else
	SELECTED_BVLIB=libbitvector.a
	SHARED_SUFFIX=
fi

OUT_NAME=libk2tree_merged${ALLOC_SUFFIX}${SHARED_SUFFIX}.a

rm -rf _tmp_merge
mkdir -p _tmp_merge
# put all libs into _tmp_merge
cp bin/${SELECTED_LIB} _tmp_merge/
cp ${CURRENT_PATH}/lib/c-bitvector/bin/${SELECTED_BVLIB} _tmp_merge/
cp ${CURRENT_PATH}/lib/c-vector/bin/libvector.a _tmp_merge/
cp ${CURRENT_PATH}/lib/c-queue/bin/libcircular_queue.a _tmp_merge/
cd _tmp_merge && \
ar -x ${SELECTED_LIB} && \
ar -x ${SELECTED_BVLIB} && \
ar -x libvector.a && \
ar -x libcircular_queue.a && \
ar -qc ${OUT_NAME} *.o && \
cp ${OUT_NAME} ../bin/
rm -rf _tmp_merge
