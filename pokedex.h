// Adiciona um pokemon na pokedex
// Considera que ja fez a busca antes
int adicionarPokemon(char* pokemon);

// Remove um pokemon na pokedex
// Considera que ja fez a busca antes
void removerPokemon(int posicaoPokemon);

// Busca um pokemon na pokedex
int buscarPokemon(char* pokemon);

// Lista os pokemons da pokedex e os salva em 'dest'
void listarPokemon(char* dest);

// Substitui o pokemon na posição 'posicao' da pokedex pelo pokemon 'pokemon'
// Considera que ja fez a busca antes
void substituirPokemon(int posicao, char* pokemon);