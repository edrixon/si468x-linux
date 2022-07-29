OUTFILE=dab

SRC=main.c shm.c si468x.c utils.c dab.c
INC=types.h dabshmem.h si468x.h si468xROM.h dabcmd.h shm.h utils.h dab.h
OBJ=main.o shm.o si468x.o utils.o dab.o
LIB=-lpigpio -lrt -lm
CFLAGS=-Wall -pthread
LDFLAGS=-pthread
CC=gcc

$(OUTFILE): $(OBJ)
	gcc $(LDFLAGS) -o $(OUTFILE) $(OBJ) $(LIB)

main.o: $(INC) main.c

shm.o: $(INC) shm.c

si468x.o: $(INC) si468x.c

utils.o: $(INC) utils.c

dab.o: $(INC) dab.c

cli.o: $(INC) cli.c

clean:
	rm -f $(OBJ)
	rm -f $(OUTFILE)
