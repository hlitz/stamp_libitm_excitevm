CXX       = g++
CXXFLAGS += -g -Wall -pthread -O3

LD       := g++
LDFLAGS  += -lpthread

CXXFLAGS += -DLIST_NO_DUPLICATES
CXXFLAGS += -DMAP_USE_RBTREE

PROG := vacation

SRCS += client.cc customer.cc manager.cc reservation.cc vacation.cc list.cc \
	pair.cc mt19937ar.cc random.cc rbtree.cc thread.cc

OBJS := ${SRCS:.cc=.o}


.PHONY: default
default: $(PROG)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(PROG)

%.o: %.cc *.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $(PROG)
