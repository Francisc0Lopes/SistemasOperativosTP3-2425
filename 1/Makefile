CC = gcc
CFLAGS = -Wall -g -pthread

BINS = philosophers

all: $(BINS)

philosophers: philosophers.o

philosophers.o: philosophers.c

clean:
	$(RM) $(BINS) *.o
