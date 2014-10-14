#
# These lines tell where to find a transactional STL... it's a bit clunky for
# now.  You MUST provide the path to where you have gcc 4.10.0 installed, and
# the path to where your build of the github mfs409/tm_stl repository
# resides.
#
GCC410INSTALL := /home/spear/gcc410/install
TMSTL_INSTALL := /home/spear/tm_stl
BITS          := 64
CXXFLAGS      += -nostdinc -I/usr/include/
CXXFLAGS      += -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/include
CXXFLAGS      += -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/include/x86_64-unknown-linux-gnu
CXXFLAGS      += -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/libsupc++
CXXFLAGS      += -I$(GCC410INSTALL)/lib/gcc/x86_64-unknown-linux-gnu/4.10.0/include
LDFLAGS       += -fgnu-tm
LDFLAGS       += -L$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/src/obj$(BITS)
LDFLAGS       += -lstdc++

#
# basic stuff for making the benchmark
#
PROG := yada

SRCS += coordinate.cc element.cc mesh.cc region.cc yada.cc tm_hacks.cc

LIBSRCS += thread.cc

OBJS := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}

LDFLAGS += -lm

include ../Makefile.common

.PHONY: test_coordinate
test_coordinate: CXXFLAGS += -DTEST_COORDINATE
test_coordinate:
	$(CC) $(CXXFLAGS) coordinate.cc -lm -o $@

.PHONY: test_element
test_element: CXXFLAGS += -DTEST_ELEMENT
test_element: LIB_SRCS := $(LIB)/{heap,list,avltree}.cc
test_element:
	$(CC) $(CXXFLAGS) element.cc coordinate.cc $(LIB_SRCS) -lm -o $@

.PHONY: test_mesh
test_mesh: CXXFLAGS += -DTEST_MESH
test_mesh: LIB_SRCS := $(LIB)/{heap,list,pair,avltree,queue,rbtree,random,mt19937ar}.cc
test_mesh:
	$(CC) $(CXXFLAGS) mesh.cc element.cc coordinate.cc $(LIB_SRCS) -lm -o $@

