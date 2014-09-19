PROG := genome

SRCS += \
	gene.c \
	genome.c \
	segments.c \
	sequencer.c \
	table.c

LIBSRCS += \
	bitmap.c \
	hash.c \
	hashtable.c \
	pair.c \
	random.c \
	list.c \
	mt19937ar.c \
	thread.c \
	vector.c

OBJS := ${SRCS:.c=.o} ${LIBSRCS:%.c=lib_%.o}

CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DCHUNK_STEP1=12

include ../Makefile.common


