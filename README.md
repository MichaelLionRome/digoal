# K2treeDyn

# Build

* Build tested on Ubuntu 18.04
* Most likely won't work on Windows, but might work with custom compilation flow, the only platform dependent code
is __builtin_popcount, can be redefined in definitions.h (with POP_COUNT(u32_input))
* Untested on OS X

`
make
`

# Test

* Required CMake 3.14

gtest will be installed with the make test-build command


`
make test-all
`


# Usage

The generated code is a static library. There are 4 alternatives to link to,
the easiest to use is bin/libk2tree_merged.a which is merged with other libraries which
this project is dependent on.

### Differences in static lib variants

* bin/libk2tree_merged_noalloc.a: Merged with dependencies, but allows for custom memory allocation (implementing memalloc.h).
* bin/libk2tree_merged.a: Comes with default memory allocation (free, malloc, calloc)
* bin/libk2tree_noalloc.a: Unmerged dependencies with abstract allocation
* bin/libk2tree.a: Unmerged dependencies with default memory allocation

There are more variants which differ in the use of the -fPIC flag to compile the dependencies

### Linking

When compiling your source code with this lib add -L{PATH_TO_BASE}/bin -l{preferred static lib} -I${PATH_TO_BASE}/include.
Having -l for example: -lk2tree_merged.

