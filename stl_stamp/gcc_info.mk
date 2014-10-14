#
# These lines tell where to find a transactional STL... it's a bit clunky for
# now.  You MUST provide the path to where you have gcc 5.0.0 installed, and
# the path to where your build of the github mfs409/tm_stl repository
# resides.
#
GCC5INSTALL := /home/spear/gcc5/install
TMSTL_INSTALL := /home/spear/tm_stl
BITS          := 64
CXXFLAGS      += -nostdinc -I/usr/include/
CXXFLAGS      += -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/include
CXXFLAGS      += -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/include/x86_64-unknown-linux-gnu
CXXFLAGS      += -I$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/libsupc++
CXXFLAGS      += -I$(GCC5INSTALL)/lib/gcc/x86_64-unknown-linux-gnu/5.0.0/include
LDFLAGS       += -fgnu-tm
LDFLAGS       += -L$(TMSTL_INSTALL)/libstdc++_tm/libstdc++-v3/src/obj$(BITS)
LDFLAGS       += -lstdc++
