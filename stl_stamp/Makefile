# Exclusively build with gcctm
BENCHS := bayes genome intruder labyrinth kmeans ssca2 vacation yada

.PHONY : clean $(BENCHS)

all: 	$(BENCHS)	

$(BENCHS):
	$(MAKE) -C $@ $@

clean:
	for i in $(BENCHS); do  \
	  $(MAKE) -C $$i clean; \
	done
# TODO better?
# TARGET=clean $(MAKE) -C bayes

