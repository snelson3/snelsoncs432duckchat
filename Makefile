CC=g++

CFLAGS=-Wall -W -g -Werror
CFLAGS=-Wall -W -g -Wno-unused


all: client server

client: client.c raw.c
	$(CC) client.c raw.c $(CFLAGS) -o client

server: server.C
	$(CC) server.C $(CFLAGS) -o server -luuid 

clean:
	rm -f client server *.o
