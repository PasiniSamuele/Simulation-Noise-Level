IDIR=utils
CC=mpicc
CFLAGS=-I$(IDIR)

LIBS=-lm -lmosquitto -lconfig

UTILS=utils/noise_sim.c utils/mqtt.c utils/config_parser.c

NPROC=2

all: main.c $(UTILS)
	$(CC) -o main ./main.c $(CFLAGS) $(UTILS) $(LIBS)

run: all
	mpirun -np $(NPROC) ./main

.PHONY: clean

clean:
	rm -rf main *.o
