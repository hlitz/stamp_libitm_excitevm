PROG := labyrinth

SRCS += \
	coordinate.c \
	grid.c \
	labyrinth.c \
	maze.c \
	router.c

LIBSRCS += \
	list.c \
	mt19937ar.c \
	pair.c \
	queue.c \
	random.c \
	thread.c \
	vector.c

OBJS := ${SRCS:.c=.o} ${LIBSRCS:%.c=lib_%.o}

CFLAGS += -DUSE_EARLY_RELEASE

LDFLAGS += -lm

include ../Makefile.common

