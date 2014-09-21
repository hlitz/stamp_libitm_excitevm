PROG := labyrinth

SRCS += \
	coordinate.cc \
	grid.cc \
	labyrinth.cc \
	maze.cc \
	router.cc

LIBSRCS += \
	list.cc \
	pair.cc \
	queue.cc \
	thread.cc \
	vector.cc

OBJS := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}

CXXFLAGS += -DUSE_EARLY_RELEASE

LDFLAGS += -lm

include ../Makefile.common

