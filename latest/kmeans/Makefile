PROG := kmeans

SRCS += \
	cluster.c \
	common.c \
	kmeans.c \
	normal.c

LIBSRCS += \
	mt19937ar.c \
	random.c \
	thread.c

OBJS := ${SRCS:.c=.o} ${LIBSRCS:%.c=lib_%.o}

# Display results of the benchmark
# CFLAGS += -DOUTPUT_TO_STDOUT

LDFLAGS += -lm

include ../Makefile.common

