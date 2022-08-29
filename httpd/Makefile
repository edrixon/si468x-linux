OUTFILE=httpd

SRC=main.c
INC=../types.h ../dabshmem.h ../shm.h  httpd.h globals.h
OBJ=main.o httpd.o httphandlers.o
OBJDAB=../shm.o
LIB=-lrt
CFLAGS=-Wall -pthread
LDFLAGS=-pthread
CC=gcc

$(OUTFILE): $(OBJ)
	gcc $(LDFLAGS) -o $(OUTFILE) $(OBJ) $(OBJDAB) $(LIB)

main.o: $(INC) main.c

httpd.o: $(INC) httpd.c

httphandlers.o: $(INC) httphandlers.c

clean:
	rm -f $(OBJ)
	rm -f $(OUTFILE)
