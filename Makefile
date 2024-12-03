CC      = gcc
CFLAGS += -Wall -g -O4
LDLIBS += -lm -lrt

all: stencil_seq stencil_mpi

clean:
	-rm stencil_seq stencil_mpi

mrproper: clean
	-rm *~
