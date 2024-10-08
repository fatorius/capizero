/*
Nesse arquivo estão definidos os parametros do funcionamento
da engine, os valores máximos para as estruturas de dados que 
afeta diretamente na quantidade de memória alocada para uso
do programa 
*/

#ifndef PARAMS
#define PARAMS

namespace Params{
    #define MAX_PLY 64
    #define MAXHASH 20000000
    #define HASHSIZE   10000

    #define PILHA_DE_LANCES 4000
    #define PILHA_DO_JOGO 2000

    #define MAGIC_HASHTABLE_SIZE 4096
};

#endif