#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

void tratarParametroIncorreto(char* comandoPrograma) {
    printf("Uso: %s <v4|v6> <porta do servidor>\n", comandoPrograma);
    printf("Exemplo: %s v4 51511\n", comandoPrograma);
    exit(EXIT_FAILURE);
}

void verificarParametros(int argc, char **argv) {
    if(argc < 3) {
        tratarParametroIncorreto(argv[0]);
    }
}

void inicializarDadosSocket(const char *protocolo, const char *portaStr, struct sockaddr_storage *dadosSocket, char* comandoPrograma) {
    if(portaStr == NULL) {
        tratarParametroIncorreto(comandoPrograma);
    }
    
    unsigned short porta = (unsigned short) atoi(portaStr); // unsigned short

    if(porta == 0) {
        tratarParametroIncorreto(comandoPrograma);
    }

    porta = htons(porta); // host to network short
    memset(dadosSocket, 0, sizeof(*dadosSocket));
    if(strcmp(protocolo, "v4") == 0) {
        struct sockaddr_in *dadosSocketv4 = (struct sockaddr_in *)dadosSocket;
        dadosSocketv4->sin_family = AF_INET;
        dadosSocketv4->sin_addr.s_addr = INADDR_ANY;
        dadosSocketv4->sin_port = port;
    } else {
        if(strcmp(protocolo, "v6") == 0) {
            struct sockaddr_in6 *dadosSocketv6 = (struct sockaddr_in6 *)storage;
            dadosSocketv6->sin6_family = AF_INET6;
            dadosSocketv6->sin6_addr = in6addr_any;
            dadosSocketv6->sin6_port = port;
        } else {
            tratarParametroIncorreto(comandoPrograma);
        }
    }
}

int main(int argc, char **argv) {
    
    verificarParametros(argc, argv);

    struct sockaddr_storage dadosSocket;
    
    inicializarDadosSocket(argv[1], argv[2], &dadosSocket, argv[0]);

    int socket;
    socket = socket(dadosSocket.ss_family, SOCK_STREAM, 0);

    if(socket == -1) {
        sairComMensagem("Erro ao iniciar o socket");
    }

    int enable = 1;
    if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
        sairComMensagem("Erro ao definir as opcoes do socket");
    }

    struct sockaddr *enderecoSocket = (struct sockaddr *)(&dadosSocket);

    if(bind(socket, enderecoSocket, sizeof(dadosSocket)) != 0) {
        sairComMensagem("Erro ao dar bind no endereço e porta para o socket");
    }

    if(listen(socket, 100) != 0) {
        sairComMensagem("Erro ao escutar por conexoes no servidor");
    }

    char enderecoStr[BUFSZ];
    converterEnderecoParaString(addr, enderecoStr, BUFSZ);
    printf("Escutando no endereço %s, esperando conexoes\n", enderecoStr);

    while (1) {
        // struct sockaddr_storage cstorage;
        // struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        // socklen_t caddrlen = sizeof(cstorage);

        // int csock = accept(s, caddr, &caddrlen);
        // if (csock == -1) {
        //     sairComMensagem("accept");
        // }

        // char caddrstr[BUFSZ];
        // converterEnderecoParaString(caddr, caddrstr, BUFSZ);
        // printf("[log] connection from %s\n", caddrstr);

        // char buf[BUFSZ];
        // memset(buf, 0, BUFSZ);
        // size_t count = recv(csock, buf, BUFSZ - 1, 0);
        // printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        // sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
        // count = send(csock, buf, strlen(buf) + 1, 0);
        // if (count != strlen(buf) + 1) {
        //     sairComMensagem("send");
        // }
        // close(csock);
    }

    exit(EXIT_SUCCESS);
}