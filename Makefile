CC=gcc
CFLAGS=-lpthread -I.

build: app

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
