CC=gcc
CFLAGS=-Wall

.PHONY: clean

all: server client

clean:
	rm counter-server counter-client

server: counter-server.c counter-common.c
	$(CC) $(CFLAGS) $(LDLIBS) counter-common.c counter-server.c -o counter-server

client: counter-client.c counter-common.c
	$(CC) $(CFLAGS) $(LDLIBS) counter-common.c counter-client.c -o counter-client
