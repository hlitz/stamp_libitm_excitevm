PROG := bayes

SRCS += \
	adtree.c \
	bayes.c \
	data.c \
	learner.c \
	net.c \
	sort.c

LIBSRCS += \
	bitmap.c \
	list.c \
	mt19937ar.c \
	queue.c \
	random.c \
	thread.c \
	vector.c

OBJS    := ${SRCS:.c=.o} ${LIBSRCS:%.c=lib_%.o}

CFLAGS  += -DLIST_NO_DUPLICATES
CFLAGS  += -DLEARNER_TRY_REMOVE
CFLAGS  += -DLEARNER_TRY_REVERSE

LDFLAGS += -lm

include ../Makefile.common

.PHONY: test_data
test_data: CFLAGS += -DTEST_DATA -O0
test_data: LIB_SRCS := ../lib/{bitmap,list,queue,random,mt19937ar,vector}.c
test_data:
	$(CC) $(CFLAGS) data.c sort.c net.c $(LIB_SRCS) -o $@

.PHONY: test_net
test_net: CFLAGS += -DTEST_NET -O0
test_net: LIB_SRCS := ../lib/{list,queue,bitmap,random,mt19937ar,vector}.c
test_net:
	$(CC) $(CFLAGS) net.c $(LIB_SRCS) -o $@

.PHONY: test_adtree
test_adtree: CFLAGS += -DTEST_ADTREE -O0
test_adtree: LIB_SRCS := ../lib/{bitmap,queue,list,random,mt19937ar,vector}.c
test_adtree:
	$(CC) $(CFLAGS) adtree.c data.c net.c sort.c $(LIB_SRCS) -o $@

.PHONY: test_learner
test_learner: CFLAGS += -DTEST_LEARNER -O0
test_learner: LIB_SRCS := ../lib/{bitmap,list,queue,random,mt19937ar,thread,vector}.c -lm
test_learner:
	$(CC) $(CFLAGS) learner.c sort.c adtree.c data.c net.c $(LIB_SRCS) -o $@

