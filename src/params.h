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

    #define DEFAULT_TT_MB 128
    #define MIN_TT_MB     1
    #define MAX_TT_MB     4096

    // Threads de busca (Lazy SMP): cada uma tem o próprio Estado (~3 MB)
    #define MAX_THREADS 64

    // Orçamento mínimo de um lance, em ms
    #define TEMPO_MINIMO_LANCE 5
    // ms que sempre ficam no relógio, para absorver a latência da GUI e do processo
    #define TEMPO_RESERVA 30

    #define PILHA_DE_LANCES 10000
    #define PILHA_DO_JOGO 2000

    #define MAGIC_HASHTABLE_SIZE 4096

    // Hash de estruturas de peões (entradas de ~40 bytes, ~2,5 MB)
    #define PAWN_HASH_BITS 16
    #define PAWN_HASH_SIZE (1 << PAWN_HASH_BITS)
};

#endif