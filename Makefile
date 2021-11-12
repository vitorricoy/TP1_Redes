all:
	gcc -Wall -c common.c
	gcc -Wall client.c -g common.o -o client
	gcc -Wall server.c -g common.o -o server

clean:
	rm common.o client server
