CC=gcc
CFLAGS=-Wall

.PHONY: clean

all: counter-server counter-client

clean:
	rm counter-server counter-client

server: counter-server.c
	${CC} ${CFLAGS} ${LDLIBS} counter-server.c -o counter-server

client: counter-client.c
	${CC} ${CFLAGS} ${LDLIBS} counter-client.c -o counter-client
