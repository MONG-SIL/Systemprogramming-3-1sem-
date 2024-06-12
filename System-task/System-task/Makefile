CC = gcc
CFLAGS = -Wall -Wextra -m64 -luser32

OBJS = main.o 

myprogram: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o myprogram

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f myprogram $(OBJS)