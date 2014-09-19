PROG := vacation

SRCS += \
	client.c \
	customer.c \
	manager.c \
	reservation.c \
	vacation.c

LIBSRCS += \
	list.c \
	pair.c \
	mt19937ar.c \
	random.c \
	rbtree.c \
	thread.c

OBJS := ${SRCS:.c=.o} ${LIBSRCS:%.c=lib_%.o}

CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DMAP_USE_RBTREE

include ../Makefile.common

