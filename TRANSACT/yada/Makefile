PROG := yada

SRCS += \
	coordinate.c \
	element.c \
	mesh.c \
	region.c \
	yada.c

LIBSRCS += \
	avltree.c \
	heap.c \
	list.c \
	mt19937ar.c \
	pair.c \
	queue.c \
	random.c \
	rbtree.c \
	thread.c \
	vector.c

OBJS := ${SRCS:.c=.o} ${LIBSRCS:%.c=lib_%.o}

CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DMAP_USE_AVLTREE
CFLAGS += -DSET_USE_RBTREE

LDFLAGS += -lm

include ../Makefile.common

.PHONY: test_coordinate
test_coordinate: CFLAGS += -DTEST_COORDINATE
test_coordinate:
	$(CC) $(CFLAGS) coordinate.c -lm -o $@

.PHONY: test_element
test_element: CFLAGS += -DTEST_ELEMENT
test_element: LIB_SRCS := $(LIB)/{heap,list,pair,avltree}.c
test_element:
	$(CC) $(CFLAGS) element.c coordinate.c $(LIB_SRCS) -lm -o $@

.PHONY: test_mesh
test_mesh: CFLAGS += -DTEST_MESH
test_mesh: LIB_SRCS := $(LIB)/{heap,list,pair,avltree,queue,rbtree,random,mt19937ar}.c
test_mesh:
	$(CC) $(CFLAGS) mesh.c element.c coordinate.c $(LIB_SRCS) -lm -o $@

