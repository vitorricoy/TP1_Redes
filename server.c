#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 512
#define TAM_POKEDEX 40

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
        dadosSocketv4->sin_port = port;
    } else {
        if(strcmp(protocolo, "v6") == 0) {
            struct sockaddr_in6 *dadosSocketv6 = (struct sockaddr_in6*)storage;
            dadosSocketv6->sin6_family = AF_INET6;
            dadosSocketv6->sin6_addr = in6addr_any;
            dadosSocketv6->sin6_port = port;
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

bool mensagemInvalida(char* mensagem) {
    int tamanhoMensagem = len(mensagem);
    int posicao;
    for(posicao = 0; posicao < tamanhoMensagem; posicao++) {
        if((!isalnum(mensagem[posicao]) && mensagem[posicao] != ' ') || (mensagem[posicao] >= 'A' && mensagem[posicao] <= 'Z')) { // letra minuscula, numero ou espaço
            return true;
        }
    }
    return false;
}

// Considera que ja fez a busca antes
int adicionarPokemon(char** pokedex, char* pokemon, int* proximaPosicao) {
    if((*proximaPosicao) == TAM_POKEDEX) {
        return -1;
    }
    pokedex[(*proximaPosicao)] = pokemon;
    (*proximaPosicao)++;
    return 0;
}

// Considera que ja fez a busca antes
void removerPokemon(char** pokedex, int posicaoPokemon, int* proximaPosicao) {
    (*proximaPosicao)--;
    int posicao;
    for(posicao = posicaoPokemon; posicao < (*proximaPosicao); posicao++) {
        pokedex[posicao] = pokedex[posicao+1];
    }
}

int buscarPokemon(char** pokedex, char* pokemon, int* proximaPosicao) {
    int posicao;
    for(posicao = 0; posicao < (*proximaPosicao); posicao++) {
        if(strcmp(pokedex[posicao], pokemon) == 0) {
            return posicao;
        }
    }
    return -1;
}

void listPokemon(char** pokedex, char* dest, int* proximaPosicao) {
    if((*proximaPosicao) == 0) {
        strcpy(dest, "none");
        return;
    }
    int posicao;
    int posicaoDest = 0;
    for(posicao = 0; posicao < (*proximaPosicao); posicao++) {
        int tamPokemon = strlen(pokedex[posicao]);
        int posicaoPokemon;
        for(posicaoPokemon = 0; posicaoPokemon < tamPokemon; posicaoPokemon++) {
            dest[posicaoDest] = pokedex[posicao][posicaoPokemon];
            posicaoDest++;
        }
        dest[posicaoDest] = ' ';
        posicaoDest++;
    }
    dest[posicaoDest] = '\0';
}

void enviarMensagem(char* mensagem, int socketCliente) {
    strcat(mensagem, "\n");
    size_t tamanhoMensagemEnviada = send(socketCliente, mensagem, strlen(mensagem), 0);
    if (count != strlen(mensagem)) {
        sairComMensagem("Erro ao enviar mensagem ao cliente");
    }
}

int main(int argc, char** argv) {
    
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

    struct sockaddr *enderecoSocket = (struct sockaddr*)(&dadosSocket);

    if(bind(socket, enderecoSocket, sizeof(dadosSocket)) != 0) {
        sairComMensagem("Erro ao dar bind no endereço e porta para o socket");
    }

    if(listen(socket, 100) != 0) {
        sairComMensagem("Erro ao escutar por conexoes no servidor");
    }

    char enderecoStr[BUFSZ];
    converterEnderecoParaString(enderecoSocket, enderecoStr, BUFSZ);
    printf("Escutando no endereço %s, esperando conexoes\n", enderecoStr);

    char* pokedex[TAM_POKEDEX];
    int proximaPosicaoPokedex = 0;

    while (1) {
        struct sockaddr_storage dadosSocketCliente;
        struct sockaddr* enderecoSocketCliente = (struct sockaddr*)(&dadosSocketCliente);
        sockeln_t tamanhoEnderecoSocketCliente = sizeof(enderecoSocketCliente);

        int socketCliente = accept(socket, enderecoSocketCliente, &tamanhoEnderecoSocketCliente);

        if(socketcliente == -1) {
            sairComMensagem("Erro ao aceitar a conexao de um cliente");
        }

        char enderecoClienteStr[BUFSZ];
        converterEnderecoParaString(enderecoSocketCliente, enderecoClienteStr, BUFSZ);
        printf("Conexão recebida de %s\n", enderecoClienteStr);

        char mensagem[BUFSZ];
        memset(mensagem, 0, BUFSZ);
        size_t tamanhoMensagem = 0;
        // Lê enquanto não terminar com \n
        while(mensagem[strlen(mensagem)-1] != '\n') {
            tamanhoMensagem+= recv(socketCliente, mensagem+tamanhoMensagem, BUFSZ-(int)tamanhoMensagem-1, 0);
        }
        mensagem[tamanhoMensagem] = '\0';
    
        printf("Recebido %d bytes: %s\n", enderecoClienteStr, (int)tamanhoMensagem, mensagem)

        if(mensagemInvalida(mensagem)) {
            // Envia erro de mensagem invalida
            char resposta[BUFSZ];
            strcpy(resposta, "invalid message");
            enviarMensagem(resposta, socketCliente);
        }

        char *p;
        bool recebeuMensagemEncerramento = false;
        for(p = strtok(s, "\n"); p != NULL; p = strtok(NULL, "\n")) {
            char operacao[BUFSZ];
            if(p == NULL) {
                // Envia erro de mensagem invalida
                char resposta[BUFSZ];
                strcpy(resposta, "invalid message");
                enviarMensagem(resposta, socketCliente);
                continue;
            }
            char* p = extrairStringAteEspacoRetornaStringDepoisDoEspaço(p, operacao);
            if(strcmp(operacao, "add") == 0) {
                char pokemon[BUFSZ];
                bool entrouUmaVez = false;
                char mensagemResposta[BUFSZ];
                memset(mensagemResposta, 0, sizeof(mensagemResposta));
                while(p != NULL) {
                    entrouUmaVez = true;
                    memset(pokemon, 0, sizeof(pokemon));
                    p = extrairStringAteEspacoRetornaStringDepoisDoEspaço(p, pokemon);
                    if(strlen(pokemon) > 10) {
                        // Envia erro de mensagem invalida
                        char resposta[BUFSZ];
                        strcpy(resposta, "invalid message");
                        strcat(mensagemResposta, resposta);
                        strcat(mensagemResposta, " ");
                    } else {
                        if(buscarPokemon(pokedex, pokemon, &proximaPosicaoPokedex) == -1) {
                            if(adicionarPokemon(pokedex, pokemon, &proximaPosicaoPokedex) == -1) {
                                // Enviar mensagem pokedex cheia
                                char resposta[BUFSZ];
                                strcpy(resposta, "limit exceeded");
                                strcat(mensagemResposta, resposta);
                                strcat(mensagemResposta, " ");
                            } else {
                                // Enviar mensagem pokemon adicionado
                                char resposta[BUFSZ];
                                sprintf(resposta, "%s added", pokemon);
                                strcat(mensagemResposta, resposta);
                                strcat(mensagemResposta, " ");
                            }
                        } else {
                            // Enviar erro pokemon existente
                            char resposta[BUFSZ];
                            sprintf(resposta, "%s already exists", pokemon);
                            strcat(mensagemResposta, resposta);
                            strcat(mensagemResposta, " ");
                        }
                    }
                }
                if(!entrouUmaVez) {
                    // Envia erro de mensagem invalida
                    char resposta[BUFSZ];
                    strcpy(resposta, "invalid message");
                    strcat(mensagemResposta, resposta);
                    strcat(mensagemResposta, " ");
                }
                enviarMensagem(mensagemResposta, socketCliente);
            } else if(strcmp(operacao, "remove") == 0) {
                char pokemon[BUFSZ];
                memset(pokemon, 0, sizeof(pokemon));
                if(p == NULL) {
                    // Envia erro de mensagem invalida
                    char resposta[BUFSZ];
                    strcpy(resposta, "invalid message");
                    enviarMensagem(resposta, socketCliente);
                    continue;
                }
                p = extrairStringAteEspacoRetornaStringDepoisDoEspaço(p, pokemon);
                if(strlen(pokemon) > 10) {
                    // Envia erro de mensagem invalida
                    char resposta[BUFSZ];
                    strcpy(resposta, "invalid message");
                    enviarMensagem(resposta, socketCliente);
                } else {
                    int posicaoPokemon = buscarPokemon(pokedex, pokemon, &proximaPosicaoPokedex);
                    if(posicaoPokemon != -1) {
                        removerPokemon(pokedex, posicaoPokemon, &proximaPosicaoPokedex);
                        // Enviar mensagem pokemon removido
                        char resposta[BUFSZ];
                        sprintf(resposta, "%s removed", pokemon);
                        enviarMensagem(resposta, socketCliente);
                    } else {
                        // Enviar erro de pokemon inexistente
                        char resposta[BUFSZ];
                        sprintf(resposta, "%s does not exist", pokemon);
                        enviarMensagem(resposta, socketCliente);
                    }
                }
            } else if(strcmp(operacao, "list") == 0) {
                char pokemonList[BUFSZ];
                listPokemon(pokedex, pokemonList, &proximaPosicaoPokedex);
                // Envia a lista de pokemons
                enviarMensagem(pokemonList, socketCliente);
            } else if(strcmp(operacao, "exchange") == 0) {
                char oldPokemon[BUFSZ];
                char newPokemon[BUFSZ];
                memset(oldPokemon, 0, sizeof(oldPokemon));
                memset(newPokemon, 0, sizeof(newPokemon));
                if(p == NULL) {
                    // Envia erro de mensagem invalida
                    char resposta[BUFSZ];
                    strcpy(resposta, "invalid message");
                    enviarMensagem(resposta, socketCliente);
                    continue;
                }
                p = extrairStringAteEspacoRetornaStringDepoisDoEspaço(p, oldPokemon);
                if(p == NULL) {
                    // Envia erro de mensagem invalida
                    char resposta[BUFSZ];
                    strcpy(resposta, "invalid message");
                    enviarMensagem(resposta, socketCliente);
                    continue;
                }
                p = extrairStringAteEspacoRetornaStringDepoisDoEspaço(p, newPokemon);
                int posicaoPokemon = buscarPokemon(pokedex, , &proximaPosicaoPokedex);
                if(posicaoPokemon != -1) {
                    pokedex[posicaoPokemon] = newPokemon;
                    // Enviar mensagem de troca
                    char resposta[BUFSZ];
                    sprintf(resposta, "%s exchanged", oldPokemon);
                    enviarMensagem(resposta, socketCliente);
                } else {
                    // Enviar erro de pokemon inexistente
                    char resposta[BUFSZ];
                    sprintf(resposta, "%s does not exist", oldPokemon);
                    enviarMensagem(resposta, socketCliente);
                }
            } else if(strcmp(operacao, "kill") == 0) {
                recebeuMensagemEncerramento = true;
                break;
            } else {
                // Envia erro de mensagem invalida
                char resposta[BUFSZ];
                strcpy(resposta, "invalid message");
                enviarMensagem(resposta, socketCliente);
            }
        }
        close(csock);
        if(recebeuMensagemEncerramento) {
            break;
        }
    }

    exit(EXIT_SUCCESS);
}