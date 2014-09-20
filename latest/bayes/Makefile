PROG := bayes

SRCS += \
	adtree.cc \
	bayes.cc \
	data.cc \
	learner.cc \
	net.cc \
	sort.cc

LIBSRCS += bitmap.cc list.cc queue.cc thread.cc vector.cc

OBJS    := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}

CXXFLAGS  += -DLIST_NO_DUPLICATES
CXXFLAGS  += -DLEARNER_TRY_REMOVE
CXXFLAGS  += -DLEARNER_TRY_REVERSE

LDFLAGS += -lm

include ../Makefile.common

.PHONY: test_data
test_data: CXXFLAGS += -DTEST_DATA -O0
test_data: LIB_SRCS := ../lib/{bitmap,list,queue,random,mt19937ar,vector}.cc
test_data:
	$(CC) $(CXXFLAGS) data.cc sort.cc net.cc $(LIB_SRCS) -o $@

.PHONY: test_net
test_net: CXXFLAGS += -DTEST_NET -O0
test_net: LIB_SRCS := ../lib/{list,queue,bitmap,random,mt19937ar,vector}.cc
test_net:
	$(CC) $(CXXFLAGS) net.cc $(LIB_SRCS) -o $@

.PHONY: test_adtree
test_adtree: CXXFLAGS += -DTEST_ADTREE -O0
test_adtree: LIB_SRCS := ../lib/{bitmap,queue,list,random,mt19937ar,vector}.cc
test_adtree:
	$(CC) $(CXXFLAGS) adtree.cc data.cc net.cc sort.cc $(LIB_SRCS) -o $@

.PHONY: test_learner
test_learner: CXXFLAGS += -DTEST_LEARNER -O0
test_learner: LIB_SRCS := ../lib/{bitmap,list,queue,random,mt19937ar,thread,vector}.cc -lm
test_learner:
	$(CC) $(CXXFLAGS) learner.cc sort.cc adtree.cc data.cc net.cc $(LIB_SRCS) -o $@

