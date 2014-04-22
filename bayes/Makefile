CC       := gcc
CFLAGS   += -g -Wall -pthread
CFLAGS   += -O3
CFLAGS   += -I$(LIB)
CPP      := g++
CPPFLAGS += $(CFLAGS)
LD       := g++
LIBS     += -lpthread

# Remove these files when doing clean
OUTPUT +=

LIB := ../lib

STM := ../../tl2

LOSTM := ../../OpenTM/lostm



PROG := bayes

SRCS += \
	adtree.c \
	bayes.c \
	data.c \
	learner.c \
	net.c \
	sort.c \
	bitmap.c \
	list.c \
	mt19937ar.c \
	queue.c \
	random.c \
	thread.c \
	vector.c

OBJS := ${SRCS:.c=.o}

CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DLEARNER_TRY_REMOVE
CFLAGS += -DLEARNER_TRY_REVERSE

CFLAGS   += -DSTM -I$(STM)
CPPFLAGS := $(CFLAGS)
LDFLAGS  += -L$(STM)
LIBS     += -ltl2

.PHONY: default
default: $(PROG)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(PROG) $(OUTPUT)

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $(PROG)

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp *.h
	$(CPP) $(CPPFLAGS) -c $< -o $@

