all:
	gcc -Wall -c common.c
	gcc -Wall -c pokedex.c
	gcc -Wall client.c -g common.o -o client
	gcc -Wall server.c -g common.o pokedex.o -o server

clean:
	rm common.o pokedex.o client server
