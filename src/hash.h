#ifndef HASH
#define HASH

#include "bitboard.h"
#include "gen.h"

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
void adicionar_hash(const int ld, const lance lc);
void adicionar_chave(const int l, const int piece, const int casa);

bool hash_lookup(const int l);

u64 obter_lock();
u64 obter_chave();

#endif