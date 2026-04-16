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
    #define HASHSIZE   10000

    // TT size is in megabytes; number of entries is derived at runtime.
    // Defaults track the UCI `option name Hash` advertisement in uci.cpp.
    #define DEFAULT_TT_MB 128
    #define MIN_TT_MB     1
    #define MAX_TT_MB     4096

    #define PILHA_DE_LANCES 4000
    #define PILHA_DO_JOGO 2000

    #define MAGIC_HASHTABLE_SIZE 4096
};

#endif