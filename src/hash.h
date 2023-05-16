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

void iniciarHash();
void adicionarChave(const int lado, const int piece, const int casa);

#endif