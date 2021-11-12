#include "common.h"
#include "pokedex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define BUFSZ 512
#define TAM_POKEDEX 40

#define SUCESSO 0
#define PROXIMA_MENSAGEM 1
#define PROXIMA_COMUNICACAO 2
#define PROXIMO_CLIENTE 3
#define ENCERRAR 4

void tratarParametroIncorreto(char* comandoPrograma) {
    printf("Uso: %s <v4|v6> <porta do servidor>\n", comandoPrograma);
    printf("Exemplo: %s v4 51511\n", comandoPrograma);
    exit(EXIT_FAILURE);
}

void verificarParametros(int argc, char** argv) {
    if(argc < 3) {
        tratarParametroIncorreto(argv[0]);
    }
}

void inicializarDadosSocket(const char* protocolo, const char* portaStr, struct sockaddr_storage* dadosSocket, char* comandoPrograma) {
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
        struct sockaddr_in *dadosSocketv4 = (struct sockaddr_in*)dadosSocket;
        dadosSocketv4->sin_family = AF_INET;
        dadosSocketv4->sin_addr.s_addr = INADDR_ANY;
        dadosSocketv4->sin_port = porta;
    } else {
        if(strcmp(protocolo, "v6") == 0) {
            struct sockaddr_in6 *dadosSocketv6 = (struct sockaddr_in6*)dadosSocket;
            dadosSocketv6->sin6_family = AF_INET6;
            dadosSocketv6->sin6_addr = in6addr_any;
            dadosSocketv6->sin6_port = porta;
        } else {
            tratarParametroIncorreto(comandoPrograma);
        }
    }
}

char* extrairStringAteEspacoRetornaStringDepoisDoEspaço(char* string, char* dest) {
    int posicao;
    int tamanhoString = strlen(string);
    for(posicao = 0; posicao < tamanhoString; posicao++) {
        if(string[posicao] == ' ') {
            dest[posicao] = '\0';
            return string+posicao+1;
        } else {
            dest[posicao] = string[posicao];
        }
    }
    dest[posicao] = '\0';
    return NULL;
}

int mensagemInvalida(char* mensagem) {
    int tamanhoMensagem = strlen(mensagem);
    int posicao;
    for(posicao = 0; posicao < tamanhoMensagem; posicao++) {
        if((!isalnum(mensagem[posicao]) && mensagem[posicao] != ' ' && mensagem[posicao] != '\n') || (mensagem[posicao] >= 'A' && mensagem[posicao] <= 'Z')) { // letra minuscula, numero ou espaço
            return 1;
        }
    }
    return 0;
}

void enviarMensagem(char* mensagem, int socketCliente) {
    strcat(mensagem, "\n");
    size_t tamanhoMensagemEnviada = send(socketCliente, mensagem, strlen(mensagem), 0);
    if (strlen(mensagem) != tamanhoMensagemEnviada) {
        sairComMensagem("Erro ao enviar mensagem ao cliente");
    }
}

int inicializarSocketServidor(struct sockaddr_storage* dadosSocket) {
    int socketServidor;
    socketServidor = socket(dadosSocket -> ss_family, SOCK_STREAM, 0);

    if(socketServidor == -1) {
        sairComMensagem("Erro ao iniciar o socket");
    }

    int enable = 1;
    if(setsockopt(socketServidor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
        sairComMensagem("Erro ao definir as opcoes do socket");
    }

    return socketServidor;
}

void escutarPorConexoes(int socketServidor, struct sockaddr_storage* dadosSocket) {
    struct sockaddr *enderecoSocket = (struct sockaddr*)(dadosSocket);

    if(bind(socketServidor, enderecoSocket, sizeof(*dadosSocket)) != 0) {
        sairComMensagem("Erro ao dar bind no endereço e porta para o socket");
    }

    if(listen(socketServidor, 100) != 0) {
        sairComMensagem("Erro ao escutar por conexoes no servidor");
    }

    char enderecoStr[BUFSZ];
    converterEnderecoParaString(enderecoSocket, enderecoStr, BUFSZ);
    printf("Escutando no endereço %s, esperando conexoes\n", enderecoStr);
}

int aceitarSocketCliente(int socketServidor) {
    struct sockaddr_storage dadosSocketCliente;
    struct sockaddr* enderecoSocketCliente = (struct sockaddr*)(&dadosSocketCliente);
    socklen_t tamanhoEnderecoSocketCliente = sizeof(enderecoSocketCliente);

    int socketCliente = accept(socketServidor, enderecoSocketCliente, &tamanhoEnderecoSocketCliente);

    if(socketCliente == -1) {
        sairComMensagem("Erro ao aceitar a conexao de um cliente");
    }

    char enderecoClienteStr[BUFSZ];
    converterEnderecoParaString(enderecoSocketCliente, enderecoClienteStr, BUFSZ);
    printf("Conexão recebida de %s\n", enderecoClienteStr);
    return socketCliente;
}

void receberMensagem(int socketCliente, char mensagem[BUFSZ]) {
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
}

int verificarMensagemInvalida(char mensagem[BUFSZ]) {
    if(mensagemInvalida(mensagem)) {
        // Envia erro de mensagem invalida
        char resposta[BUFSZ+20];
        strcpy(resposta, "invalid message");
        enviarMensagem(resposta, socketCliente);
        return 0;
    }
    return 1;
}

void enviaErroMensagemInvalida(int socketCliente) {
    // Envia erro de mensagem invalida
    char resposta[BUFSZ+20];
    strcpy(resposta, "invalid message");
    enviarMensagem(resposta, socketCliente);
}

void enviaErroPokemonInexistente(int socketCliente, char pokemon[BUFSZ]) {
    char resposta[BUFSZ+20];
    sprintf(resposta, "%s does not exist", pokemon);
    enviarMensagem(resposta, socketCliente);
}

void enviaErroPokemonJaExistente(int socketCliente, char pokemon[BUFSZ]) {
    char resposta[BUFSZ+20];
    sprintf(resposta, "%s already exists", pokemon);
    enviarMensagem(resposta, socketCliente);
}

int processarAdd(char* mensagem, int socketCliente) {
    char pokemon[BUFSZ];
    int entrouUmaVez = 0;
    char mensagemResposta[BUFSZ];
    memset(mensagemResposta, 0, sizeof(mensagemResposta));
    while(mensagem != NULL) {
        entrouUmaVez = 1;
        memset(pokemon, 0, sizeof(pokemon));
        mensagem = extrairStringAteEspacoRetornaStringDepoisDoEspaço(mensagem, pokemon);
        if(strlen(pokemon) > 10) {
            // Envia erro de mensagem invalida
            char resposta[BUFSZ+20];
            strcpy(resposta, "invalid message");
            strcat(mensagemResposta, resposta);
            strcat(mensagemResposta, " ");
        } else {
            if(buscarPokemon(pokemon) == -1) {
                if(adicionarPokemon(pokemon) == -1) {
                    // Enviar mensagem pokedex cheia
                    char resposta[BUFSZ+20];
                    strcpy(resposta, "limit exceeded");
                    strcat(mensagemResposta, resposta);
                    strcat(mensagemResposta, " ");
                } else {
                    // Enviar mensagem pokemon adicionado
                    char resposta[BUFSZ+20];
                    sprintf(resposta, "%s added", pokemon);
                    strcat(mensagemResposta, resposta);
                    strcat(mensagemResposta, " ");
                }
            } else {
                // Enviar erro pokemon existente
                char resposta[BUFSZ+20];
                sprintf(resposta, "%s already exists", pokemon);
                strcat(mensagemResposta, resposta);
                strcat(mensagemResposta, " ");
            }
        }
    }
    if(!entrouUmaVez) {
        // Envia erro de mensagem invalida
        char resposta[BUFSZ+20];
        strcpy(resposta, "invalid message");
        strcat(mensagemResposta, resposta);
        strcat(mensagemResposta, " ");
    }
    mensagemResposta[strlen(mensagemResposta)-1] = '\0'; // Remove o espaço extra
    enviarMensagem(mensagemResposta, socketCliente);
    return PROXIMA_MENSAGEM;
}

int processarRemove(char* mensagem, int socketCliente) {
    char pokemon[BUFSZ];
    memset(pokemon, 0, sizeof(pokemon));
    if(mensagem == NULL) {
        // Busca a proxima mensagem
        enviaErroMensagemInvalida(socketCliente);
        return PROXIMA_MENSAGEM;
    }
    mensagem = extrairStringAteEspacoRetornaStringDepoisDoEspaço(mensagem, pokemon);
    if(strlen(pokemon) > 10) {
        enviaErroMensagemInvalida(socketCliente);
    } else {
        int posicaoPokemon = buscarPokemon(pokemon);
        if(posicaoPokemon != -1) {
            removerPokemon(posicaoPokemon);
            // Enviar mensagem pokemon removido
            char resposta[BUFSZ+20];
            sprintf(resposta, "%s removed", pokemon);
            enviarMensagem(resposta, socketCliente);
        } else {
            // Enviar erro de pokemon inexistente
            enviaErroPokemonInexistente(socketCliente, pokemon);
        }
    }
    return PROXIMA_MENSAGEM;
}

int processarList(int socketCliente) {
    char pokemonList[BUFSZ];
    listarPokemon(pokemonList);
    // Envia a lista de pokemons
    enviarMensagem(pokemonList, socketCliente);
    return PROXIMA_MENSAGEM;
}

int processarExchange(char* mensagem, int socketCliente) {
    char pokemonAntigo[BUFSZ];
    char novoPokemon[BUFSZ];
    memset(pokemonAntigo, 0, sizeof(pokemonAntigo));
    memset(novoPokemon, 0, sizeof(novoPokemon));
    if(mensagem == NULL) {
        enviaErroMensagemInvalida(socketCliente);
        return PROXIMA_MENSAGEM;
    }
    mensagem = extrairStringAteEspacoRetornaStringDepoisDoEspaço(mensagem, pokemonAntigo);
    if(mensagem == NULL) {
        enviaErroMensagemInvalida(socketCliente);
        return PROXIMA_MENSAGEM;
    }
    mensagem = extrairStringAteEspacoRetornaStringDepoisDoEspaço(mensagem, novoPokemon);
    int posicaoPokemonAntigo = buscarPokemon(pokemonAntigo);
    int posicaoPokemonNovo = buscarPokemon(novoPokemon);
    if(posicaoPokemonAntigo != -1 && posicaoPokemonNovo == -1) {
        substituirPokemon(posicaoPokemonAntigo, novoPokemon);
        // Enviar mensagem de troca
        char resposta[BUFSZ+20];
        sprintf(resposta, "%s exchanged", pokemonAntigo);
        enviarMensagem(resposta, socketCliente);
    } else {
        if(posicaoPokemonNovo == -1) {
            // Enviar erro de pokemon inexistente
            enviaErroPokemonInexistente(socketCliente, pokemonAntigo);
        } else {
            // Enviar erro pokemon existente
            enviaErroPokemonJaExistente(socketCliente, novoPokemon);
        }
    }
    return PROXIMA_MENSAGEM;
}

int tratarMensagemRecebida(char* mensagem, int socketCliente) {
    char operacao[BUFSZ];
    if(mensagem == NULL) {
        enviaErroMensagemInvalida(socketCliente);
        return PROXIMA_MENSAGEM;
    }
    printf("Processando a mensagem %s\n", mensagem);
    mensagem = extrairStringAteEspacoRetornaStringDepoisDoEspaço(mensagem, operacao);
    if(strcmp(operacao, "add") == 0) {
        return processarAdd(mensagem, socketCliente);
    } else if(strcmp(operacao, "remove") == 0) {
        return processarRemove(mensagem, socketCliente);
    } else if(strcmp(operacao, "list") == 0) {
        return processarList(socketCliente);
    } else if(strcmp(operacao, "exchange") == 0) {
        return processarExchange(mensagem, socketCliente);
    } else if(strcmp(operacao, "kill") == 0) {
        return ENCERRAR;
    } else {
        // Comando errado
        return PROXIMO_CLIENTE;
    }
}

int tratarMensagensRecebidas(char mensagem[BUFSZ]) {
    int errouComando = 0;
    char *parteMensagem;
    for(parteMensagem = strtok(mensagem, "\n"); parteMensagem != NULL; parteMensagem = strtok(NULL, "\n")) {
        int retorno = tratarMensagemRecebida(parteMensagem);
        if(retorno == PROXIMA_COMUNICACAO || retorno == PROXIMO_CLIENTE || retorno == ENCERRAR) {
            return retorno;
        }
    }
    return PROXIMA_COMUNICACAO;
}

int receberETratarMensagemCliente(int socketCliente) {
    char mensagem[BUFSZ];
    
    receberMensagem(socketCliente, mensagem);

    if(strlen(mensagem) == 0) {
        return PROXIMO_CLIENTE;
    }

    // TODO: Alterar para imprimir apenas a mensagem
    printf("Recebido %d bytes: %s\n", (int)tamanhoMensagem, mensagem);

    if(verificarMensagemInvalida(mensagem) == 0) {
        return PROXIMA_COMUNICACAO;
    }

    printf("Mensagem valida\n");

    int retorno = tratarMensagensRecebidas(mensagem);
    
    if(retorno == ENCERRAR || retorno == PROXIMO_CLIENTE) {
        return retorno;
    }

    return PROXIMA_COMUNICACAO;
}

int tratarConexaoCliente(int socketServidor) {
    int socketCliente = aceitarSocketCliente(socketServidor);
    int recebeuMensagemEncerramento = 0;
    while(1) {
        int retorno = receberETratarMensagemCliente(socketCliente);
        if(retorno == PROXIMO_CLIENTE || retorno == ENCERRAR) {
            return retorno;
        }
    }
    close(socketCliente);
    return PROXIMO_CLIENTE;
}

void esperarPorConexoesCliente(int socketServidor) {
    while (1) {
        int retorno = tratarConexaoCliente(socketServidor);
        if(retorno == ENCERRAR) {
            return;
        }
    }
}

int main(int argc, char** argv) {
    
    verificarParametros(argc, argv);

    struct sockaddr_storage dadosSocket;
    
    inicializarDadosSocket(argv[1], argv[2], &dadosSocket, argv[0]);

    int socketServidor = inicializarSocketServidor(&dadosSocket);

    escutarPorConexoes(socketServidor, &dadosSocket);

    esperarPorConexoesCliente(socketServidor);

    exit(EXIT_SUCCESS);
}