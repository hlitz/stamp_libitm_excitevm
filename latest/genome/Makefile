PROG := genome

SRCS += \
	gene.cc \
	genome.cc \
	segments.cc \
	sequencer.cc \
	table.cc

LIBSRCS += \
	bitmap.cc \
	hash.cc \
	hashtable.cc \
	pair.cc \
	list.cc \
	thread.cc \
	vector.cc

OBJS := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}

CXXFLAGS += -DLIST_NO_DUPLICATES
CXXFLAGS += -DCHUNK_STEP1=12

include ../Makefile.common


