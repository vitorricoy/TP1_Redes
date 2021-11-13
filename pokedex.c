#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSZ 512
#define TAM_POKEDEX 40

char pokedex[TAM_POKEDEX][BUFSZ];
int proximaPosicao = 0;

// Considera que ja fez a busca antes
int adicionarPokemon(char* pokemon) {
    if(proximaPosicao == TAM_POKEDEX) { // Pokedex cheia
        return -1;
    }
    strcpy(pokedex[proximaPosicao], pokemon);
    proximaPosicao++;
    return 0;
}

// Considera que ja fez a busca antes, portanto 'posicaoPokemon' é uma posição válida
void removerPokemon(int posicaoPokemon) {
    proximaPosicao--;
    int posicao;
    for(posicao = posicaoPokemon; posicao < proximaPosicao; posicao++) {
        strcpy(pokedex[posicao], pokedex[posicao+1]);
    }
}

int buscarPokemon(char* pokemon) {
    int posicao;
    for(posicao = 0; posicao < proximaPosicao; posicao++) {
        if(strcmp(pokedex[posicao], pokemon) == 0) {
            return posicao;
        }
    }
    return -1; // Pokemon não encontrado
}

void listarPokemon(char* dest) {
    if(proximaPosicao == 0) { // Se a pokedex está vazia retorna 'none'
        strcpy(dest, "none");
        return;
    }
    int posicao;
    int posicaoDest = 0;
    for(posicao = 0; posicao < proximaPosicao; posicao++) {
        int tamPokemon = strlen(pokedex[posicao]);
        // Copia o nome do pokemon para 'dest' seguido de um espaço
        int posicaoPokemon;
        for(posicaoPokemon = 0; posicaoPokemon < tamPokemon; posicaoPokemon++) {
            dest[posicaoDest] = pokedex[posicao][posicaoPokemon];
            posicaoDest++;
        }
        dest[posicaoDest] = ' ';
        posicaoDest++;
    }
    // Remove o último espaço adicionado
    dest[posicaoDest-1] = '\0';
}

// Considera que ja fez a busca antes, portanto 'posicao' é uma posição válida
void substituirPokemon(int posicao, char* pokemon) {
    strcpy(pokedex[posicao], pokemon);
}