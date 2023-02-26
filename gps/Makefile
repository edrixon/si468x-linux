CC=gcc
CFLAGS=-Wall
LIB=-lm -lgps
LDFLAGS=-pthread
OBJS=dabgps.o ../shm.o
INC=types.h
EXEC=dabgps

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXEC) $(OBJS) $(LIB)

%.o: %.c $(INC)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(EXEC)
