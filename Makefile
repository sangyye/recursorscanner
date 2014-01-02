# Makefile for recursorscanner

CC=gcc
CFLAGS=-c -Wall -std=gnu99

all: recursorscanner

recursorscanner: main.o dns.o
	$(CC) main.o dns.o -lm -lpthread -o recursorscanner 

main.o:
	$(CC) $(CFLAGS) main.c

dns.o:
	$(CC) $(CFLAGS) dns.c

clean:
	rm -f *.o
	rm recursorscanner

install:
	cp recursorscanner /usr/local/bin/recursorscanner

