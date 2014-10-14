include ../gcc_info.mk

PROG := genome
SRCS += gene.cc genome.cc segments.cc sequencer.cc table.cc
LIBSRCS += thread.cc
OBJS := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}
CXXFLAGS += -DLIST_NO_DUPLICATES
CXXFLAGS += -DCHUNK_STEP1=12

include ../Makefile.common


