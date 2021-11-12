#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 512

void tratarParametroIncorreto(char* comandoPrograma) {
    printf("Uso: %s <ip do servidor> <porta do servidor>\n", comandoPrograma);
    printf("Exemplo: %s 127.0.0.1 51511\n", comandoPrograma);
    exit(EXIT_FAILURE);
}

void verificarParametros(int argc, char **argv) {
    if(argc < 3) {
        tratarParametroIncorreto(argv[0]);
    }
}

void inicializarDadosSocket(const char *enderecoStr, const char *portaStr, struct sockaddr_storage *dadosSocket, char* comandoPrograma) {
    if(enderecoStr == NULL || portaStr == NULL) {
        tratarParametroIncorreto(comandoPrograma);
    }

    unsigned short porta = (unsigned short)atoi(portaStr); // unsigned short
    if(porta == 0) {
        tratarParametroIncorreto(comandoPrograma);
    }

    porta = htons(porta); // host to network short

    struct in_addr inaddr4; // 32-bit IP address
    if(inet_pton(AF_INET, enderecoStr, &inaddr4)) {
        struct sockaddr_in *dadosSocketv4 = (struct sockaddr_in*)dadosSocket;
        dadosSocketv4->sin_family = AF_INET;
        dadosSocketv4->sin_port = porta;
        dadosSocketv4->sin_addr = inaddr4;
    } else {
        struct in6_addr inaddr6; // 128-bit IPv6 address
        if (inet_pton(AF_INET6, enderecoStr, &inaddr6)) {
            struct sockaddr_in6 *dadosSocketv6 = (struct sockaddr_in6*)dadosSocket;
            dadosSocketv6->sin6_family = AF_INET6;
            dadosSocketv6->sin6_port = porta;
            memcpy(&(dadosSocketv6->sin6_addr), &inaddr6, sizeof(inaddr6));
        } else {
            tratarParametroIncorreto(comandoPrograma);
        }
    }
}


int main(int argc, char **argv) {

	verificarParametros(argc, argv);
    struct sockaddr_storage dadosSocket;
    inicializarDadosSocket(argv[1], argv[2], &dadosSocket, argv[0]);

    int socketCliente;
    socketCliente = socket(dadosSocket.ss_family, SOCK_STREAM, 0);
    if(socketCliente == -1) {
        sairComMensagem("Erro ao iniciar o socket");
    }

    struct sockaddr *enderecoSocket = (struct sockaddr*)(&dadosSocket);
    if(connect(socketCliente, enderecoSocket, sizeof(dadosSocket)) != 0) {
        sairComMensagem("Erro ao conectar no servidor");
    }

    char enderecoStr[BUFSZ];
    converterEnderecoParaString(enderecoSocket, enderecoStr, BUFSZ);

    printf("Conectado ao endereco %s\n", enderecoStr);

    char mensagem[BUFSZ];
    memset(mensagem, 0, sizeof(mensagem));
    while(1) {
        fgets(mensagem, BUFSZ-1, stdin);
        size_t tamanhoMensagemEnviada = send(socketCliente, mensagem, strlen(mensagem), 0);

        if (strlen(mensagem) != tamanhoMensagemEnviada) {
            sairComMensagem("Erro ao enviar mensagem");
        }

        memset(mensagem, 0, BUFSZ);
        size_t tamanhoMensagem = 0;
        // Lê enquanto não terminar com \n
        do {
            size_t tamanhoLidoAgora = recv(socketCliente, mensagem+tamanhoMensagem, BUFSZ-(int)tamanhoMensagem-1, 0);
            if(tamanhoLidoAgora == 0) {
                break;
            }
            tamanhoMensagem += tamanhoLidoAgora;
        }while(mensagem[strlen(mensagem)-1] != '\n');

        mensagem[tamanhoMensagem] = '\0';

        if(tamanhoMensagem == 0) {
            // Conexão caiu
            exit(EXIT_SUCCESS);
        }

        printf("%s", mensagem);
        memset(mensagem, 0, sizeof(mensagem));
    }
    send(socketCliente, "disconnect\n", 11, 0);
	exit(EXIT_SUCCESS);
}