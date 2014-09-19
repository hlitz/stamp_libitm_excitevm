PROG := yada

SRCS += \
	coordinate.cc \
	element.cc \
	mesh.cc \
	region.cc \
	yada.cc

LIBSRCS += \
	avltree.cc \
	heap.cc \
	list.cc \
	pair.cc \
	queue.cc \
	rbtree.cc \
	thread.cc \
	vector.cc

OBJS := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}

CXXFLAGS += -DLIST_NO_DUPLICATES
CXXFLAGS += -DMAP_USE_AVLTREE
CXXFLAGS += -DSET_USE_RBTREE

LDFLAGS += -lm

include ../Makefile.common

.PHONY: test_coordinate
test_coordinate: CXXFLAGS += -DTEST_COORDINATE
test_coordinate:
	$(CC) $(CXXFLAGS) coordinate.cc -lm -o $@

.PHONY: test_element
test_element: CXXFLAGS += -DTEST_ELEMENT
test_element: LIB_SRCS := $(LIB)/{heap,list,pair,avltree}.cc
test_element:
	$(CC) $(CXXFLAGS) element.cc coordinate.cc $(LIB_SRCS) -lm -o $@

.PHONY: test_mesh
test_mesh: CXXFLAGS += -DTEST_MESH
test_mesh: LIB_SRCS := $(LIB)/{heap,list,pair,avltree,queue,rbtree,random,mt19937ar}.cc
test_mesh:
	$(CC) $(CXXFLAGS) mesh.cc element.cc coordinate.cc $(LIB_SRCS) -lm -o $@

