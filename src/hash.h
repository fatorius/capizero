#ifndef HASH
#define HASH

#include "bitboard.h"

const u64 MAXHASH =  5000000;
const u64 HASHSIZE = 5000000;

struct hashp{
    u64 hashlock;
    int inicio;
    int dest;
    int num;
};

extern u64 chaveAtual, lockAtual;

void iniciar_hash();
void adicionar_chave(const int l, const int piece, const int casa);

#endif