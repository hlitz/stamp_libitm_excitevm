include ../gcc_info.mk

PROG    := labyrinth
SRCS    += coordinate.cc grid.cc labyrinth.cc maze.cc router.cc
LIBSRCS += thread.cc
OBJS    := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}
LDFLAGS += -lm

include ../Makefile.common

