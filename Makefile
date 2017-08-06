CC=gcc
PTFLAG=-pthread
DEBGFLAGS=-g
LFLAGS=-lm -lpthread
CFLAGS=-std=c99 -Wall
VPATH=utils

OBJECTS = ArrayList.o HashMap.o LinkedList.o String.o Utils.o

all: TClient TServer TClientDummy

TServer: $(OBJECTS) TServer.o
	$(CC) $(CFLAGS) -o bin/$@ $(addprefix bin/,$^) $(LFLAGS)

TClient: $(OBJECTS) TClient.o
	$(CC) $(CFLAGS) -o bin/$@ $(addprefix bin/,$^) $(LFLAGS)

TClientDummy: $(OBJECTS) TClientDummy.o
	$(CC) $(CFLAGS) -o bin/$@ $(addprefix bin/,$^) $(LFLAGS)

%.o: %.c $(wildcard %.h)
	$(CC) $(CFLAGS) $(PTFLAG) $(DEBGFLAGS) -c $< -o bin/$@

clean:
	find . -name '*.o' -delete
