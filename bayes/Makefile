CXX       = g++
CXXFLAGS += -g -Wall -pthread -O3 -fgnu-tm

LD       := g++
LDFLAGS  += -lpthread -litm

CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DLEARNER_TRY_REMOVE
CFLAGS += -DLEARNER_TRY_REVERSE

PROG := bayes

SRCS += adtree.cc bayes.cc data.cc learner.cc net.cc sort.cc bitmap.cc \
	list.cc mt19937ar.cc queue.cc random.cc thread.cc vector.cc

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
