CC=gcc
CFLAGS=-Wall
INC=mcast.h

all: mcast-rx mcast-tx

mcast-rx: mcast-rx.c mcast.h
	$(CC) $(CFLAGS) -o mcast-rx mcast-rx.c

mcast-tx: mcast-tx.c mcast.h
	$(CC) $(CFLAGS) -o mcast-tx mcast-tx.c

clean:
	rm -f mcast-rx mcast-tx
