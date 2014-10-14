include ../gcc_info.mk

PROG := kmeans
SRCS += cluster.cc common.cc kmeans.cc normal.cc
LIBSRCS += thread.cc
OBJS := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}
# Display results of the benchmark
# CXXFLAGS += -DOUTPUT_TO_STDOUT
LDFLAGS += -lm

include ../Makefile.common

