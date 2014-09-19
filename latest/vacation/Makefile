PROG := vacation

SRCS += client.cc customer.cc manager.cc reservation.cc vacation.cc

LIBSRCS += list.cc pair.cc rbtree.cc thread.cc

OBJS := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}

CXXFLAGS += -DLIST_NO_DUPLICATES
CXXFLAGS += -DMAP_USE_RBTREE

include ../Makefile.common

