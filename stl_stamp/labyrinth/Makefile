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
PROG    := labyrinth
SRCS    += coordinate.cc grid.cc labyrinth.cc maze.cc router.cc
LIBSRCS += list.cc pair.cc queue.cc thread.cc
OBJS    := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}
LDFLAGS += -lm

include ../Makefile.common

