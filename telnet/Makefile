DAB=..
CC=gcc
CFLAGS=-Wall
LIB=$(DAB)/shm.o -lrt -lgps
LDFLAGS=-pthread
OBJS=main.o cli.o telnetd.o commands.o
INC=$(DAB)/types.h $(DAB)/dabshmem.h $(DAB)/dabcmd.h $(DAB)/shm.h \
    cli.h telnetd.h commands.h globals.h
EXEC=telnetd

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXEC) $(OBJS) $(LIB)

%.o: %.c $(INC)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(EXEC)
