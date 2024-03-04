CC = gcc
CFLAGS = -Wall -Werror
PIGPIOFLAGS = -pthread -lpigpio -lrt

SRCS = main.c common.c
OBJS = $(SRCS:.c=.o)

#all: run-operation

#run-operation: $(OBJS)
#	$(CC) $(CFLAGS) $(PIGPIOFLAGS) $(OBJS) -o run-operation

#main.o: main.c common.h common.c
#	$(CC) $(CFLAGS) -c main.c

#common.o: common.c common.h
#	$(CC) $(CFLAGS) -c common.c
#
#database.o: database.c database.h
#	$(CC) $(CFLAGS) -c database.c

#clean:
#	rm -f $(OBJS) run-operation

run-operation: main.c common.c common.h
	gcc -Wall -pthread -o run-operation common.c database.c main.c -lpigpio -lrt -lm -l sqlite3

clean:
	rm -f run-operation
