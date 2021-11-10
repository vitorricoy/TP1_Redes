#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

void sairComMensagem(const char *msg);

void converterEnderecoParaString(const struct sockaddr *endereco, char *string, size_t tamanhoString);