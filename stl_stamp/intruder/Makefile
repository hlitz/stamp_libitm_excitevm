#
# These lines tell where to find a transactional STL... it's a bit clunky for
# now.  You MUST provide the path to where you have gcc 4.10.0 installed, and
# the path to where your build of the github mfs409/tm_stl repository
# resides.
#
GCC410INSTALL := /home/spear/gcc410/install
TMSTL_INSTALL := /home/spear/tm_stl
BITS          := 64
CXXFLAGS      += -nostdinc -I/usr/include/
CXXFLAGS      += -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/include
CXXFLAGS      += -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/include/x86_64-unknown-linux-gnu
CXXFLAGS      += -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/libsupc++
CXXFLAGS      += -I$(GCC410INSTALL)/lib/gcc/x86_64-unknown-linux-gnu/4.10.0/include
LDFLAGS       += -fgnu-tm
LDFLAGS       += -L$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/src/obj$(BITS)
LDFLAGS       += -lstdc++

#
# basic stuff for making the benchmark
#
PROG := intruder
SRCS += decoder.cc detector.cc dictionary.cc intruder.cc packet.cc \
        preprocessor.cc stream.cc
LIBSRCS += pair.cc queue.cc rbtree.cc thread.cc
OBJS := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}

CXXFLAGS += -DMAP_USE_RBTREE

include ../Makefile.common

.PHONY: test_decoder
test_decoder: CXXFLAGS += -DTEST_DECODER -O0
test_decoder: LIB_SRCS := $(LIB)/{list,mt19937ar,queue,random,rbtree}.cc
test_decoder:
	$(CC) $(CXXFLAGS) decoder.cc packet.cc $(LIB_SRCS) -o $@

.PHONY: test_detector
test_detector: CXXFLAGS += -DTEST_DETECTOR -O0
test_detector:
	$(CC) $(CXXFLAGS) detector.cc dictionary.cc preprocessor.cc $(LIB_SRCS) -o $@

.PHONY: test_dictionary
test_dictionary: CXXFLAGS += -DTEST_DICTIONARY -O0
test_dictionary:
	$(CC) $(CXXFLAGS) dictionary.cc $(LIB_SRCS) -o $@

.PHONY: test_preprocessor
test_preprocessor: CXXFLAGS += -DTEST_PREPROCESSOR -O0
test_preprocessor:
	$(CC) $(CXXFLAGS) preprocessor.cc -o $@

.PHONY: test_stream
test_stream: CXXFLAGS += -DTEST_STREAM -O0
test_stream: LIB_SRCS := $(LIB)/{mt19937ar,pair,queue,random,rbtree}.cc
test_stream:
	$(CC) $(CXXFLAGS) stream.cc detector.cc dictionary.cc preprocessor.cc $(LIB_SRCS) -o $@

