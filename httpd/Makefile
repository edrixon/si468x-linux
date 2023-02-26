DAB=..
CC=gcc
CFLAGS=-Wall
LIB=$(DAB)/shm.o -lrt
LDFLAGS=-pthread
OBJS=main.o httpd.o httphandlers.o
INC=$(DAB)/types.h $(DAB)/dabshmem.h $(DAB)/shm.h httpd.h globals.h
EXEC=httpd

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXEC) $(OBJS) $(LIB)

%.o: %.c $(INC)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(EXEC)
