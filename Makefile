CC=gcc
CFLAGS=-Wall
LIB=-lpigpio -lrt -lm -lgps
LDFLAGS=-pthread
OBJS=dab.o dablogger.o main.o shm.o si468x.o utils.o timers.o buzzer.o
INC=dab.h dabcmd.h dablogger.h dabshmem.h globals.h shm.h \
    si468x.h si468xROM.h types.h utils.h timers.h buzzer.h
EXEC=dab

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXEC) $(OBJS) $(LIB)

%.o: %.c $(INC)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(EXEC)
