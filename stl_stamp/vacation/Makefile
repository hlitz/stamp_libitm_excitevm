include ../gcc_info.mk

PROG    := vacation
SRCS    += client.cc customer.cc manager.cc reservation.cc vacation.cc
LIBSRCS += thread.cc
OBJS    := ${SRCS:.cc=.o} ${LIBSRCS:%.cc=lib_%.o}

include ../Makefile.common

