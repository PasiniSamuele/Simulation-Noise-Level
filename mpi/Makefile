IDIR=utils
CC=mpicc
CFLAGS=-I$(IDIR)

ODIR=obj
UDIR=utils/

LIBS=-lm -lmosquitto -lconfig

all: main
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

main:  main.o noise_sim.o mqtt.o config_parser.o
	$(CC) $(CFLAGS) -o main noise_sim.o mqtt.o config_parser.o $(LIBS)

main.o:  main.c $(UDIR)noise_sim.h $(UDIR)mqtt.h $(UDIR)config_parser.h
	$(CC) $(CFLAGS) -c main.c

noise_sim.o:  $(UDIR)noise_sim.c $(UDIR)noise_sim.h 
	$(CC) $(CFLAGS) -c $(UDIR)noise_sim.c

mqtt.o:  $(UDIR)mqtt.c $(UDIR)mqtt.h 
	$(CC) $(CFLAGS) -c $(UDIR)mqtt.c

config_parser.o:  $(UDIR)config_parser.c $(UDIR)config_parser.h 
	$(CC) $(CFLAGS) -c $(UDIR)config_parser.c


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ 