#
# These lines tell where to find a transactional STL... it's a bit clunky for
# now.  You MUST provide the path to where you have gcc 4.10.0 installed, and
# the path to where your build of the github mfs409/tm_stl repository
# resides.
#
GCC410INSTALL := /home/spear/gcc410/install
TMSTL_INSTALL := /home/spear/tm_stl
BITS := 64
CXXFLAGS += -nostdinc -I/usr/include/ -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/include        \
                 -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/include/x86_64-unknown-linux-gnu    \
                 -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/libsupc++                           \
                 -I$(GCC410INSTALL)/lib/gcc/x86_64-unknown-linux-gnu/4.10.0/include \
                 -DUSE_TM -pthread

LDFLAGS  += -fgnu-tm -L$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/src/obj$(BITS) -lstdc++ -pthread

#
# basic stuff for making the benchmark
#
PROG := vacation

SRCS += client.cc customer.cc manager.cc reservation.cc vacation.cc

LIBSRCS += thread.cc

OBJS := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}

CXXFLAGS += -DLIST_NO_DUPLICATES
CXXFLAGS += -DMAP_USE_RBTREE

include ../Makefile.common

