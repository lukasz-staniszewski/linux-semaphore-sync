CC = gcc
LIBS = -lpthread

soi3: soi3.c
	$(CC) soi3.c $(LIBS) -o soi3

clean:
	rm -f *.o soi3