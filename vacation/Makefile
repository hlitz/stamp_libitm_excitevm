# ==============================================================================
#
# Makefile.stm
#
# ==============================================================================


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


CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DMAP_USE_RBTREE

PROG := vacation

SRCS += \
	client.c \
	customer.c \
	manager.c \
	reservation.c \
	vacation.c \
	list.c \
	pair.c \
	mt19937ar.c \
	random.c \
	rbtree.c \
	thread.c 

OBJS := ${SRCS:.c=.o}

CFLAGS   += -DSTM -I$(STM)
CPPFLAGS := $(CFLAGS)
LDFLAGS  += -L$(STM)
LIBS     +=

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
