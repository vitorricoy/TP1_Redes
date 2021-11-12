all:
	gcc -Wall -c common.c
	gcc -Wall cliente.c -g common.o -o cliente
	gcc -Wall servidor.c -g common.o -o servidor

clean:
	rm common.o cliente servidor
