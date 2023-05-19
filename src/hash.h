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
extern int hash_inicio, hash_destino;

extern uint64_t colisoes;

void iniciar_hash();
void liberar_memoria();
void adicionar_chave(const int l, const int piece, const int casa);

bool hash_lookup(const int l);

u64 obter_lock();
u64 obter_chave();

#endif