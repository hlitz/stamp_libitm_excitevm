PROG := ssca2

SRCS += \
	alg_radix_smp.cc \
	computeGraph.cc \
	createPartition.cc \
	cutClusters.cc \
	findSubGraphs.cc \
	genScalData.cc \
	getStartLists.cc \
	getUserParameters.cc \
	globals.cc \
	ssca2.cc

LIBSRCS += thread.cc

OBJS := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}

#CXXFLAGS += -DUSE_PARALLEL_DATA_GENERATION
#CXXFLAGS += -DWRITE_RESULT_FILES
CXXFLAGS += -DENABLE_KERNEL1
#CXXFLAGS += -DENABLE_KERNEL2 -DENABLE_KERNEL3
#CXXFLAGS += -DENABLE_KERNEL4

LDFLAGS += -lm

include ../Makefile.common

