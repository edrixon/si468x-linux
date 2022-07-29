OUTFILE=telnetd

SRC=main.c
INC=../types.h ../dabshmem.h ../dabcmd.h ../shm.h cli.h telnetd.h commands.h
OBJ=main.o cli.o telnetd.o commands.o
OBJDAB=../shm.o
LIB=-lrt
CFLAGS=-Wall -pthread
LDFLAGS=-pthread
CC=gcc

$(OUTFILE): $(OBJ)
	gcc $(LDFLAGS) -o $(OUTFILE) $(OBJ) $(OBJDAB) $(LIB)

main.o: $(INC) main.c

cli.o: $(INC) cli.c

telnetd.o: $(INC) telnetd.c

commands.o: $(INC) commands.c

clean:
	rm -f $(OBJ)
	rm -f $(OUTFILE)
