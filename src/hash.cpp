#include "hash.h"

#include <cstdlib>

#include "consts.h"

u64 hash[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
u64 lock[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];

u64 chaveAtual, lockAtual;

hashp *hashpos[LADOS];

int aleatorio(const int x){
    return rand() % x;
}

void iniciar_hash(){
    int piece, casa;

    for (piece = 0; piece < TIPOS_DE_PIECES; piece++){
        for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
            hash[BRANCAS][piece][casa] = aleatorio(HASHSIZE);
            hash[PRETAS][piece][casa] = aleatorio(HASHSIZE);
            lock[BRANCAS][piece][casa] = aleatorio(HASHSIZE);
            lock[PRETAS][piece][casa] = aleatorio(HASHSIZE);
        }
    }

    hashpos[BRANCAS] = new hashp[MAXHASH];
    hashpos[PRETAS] = new hashp[MAXHASH];
}

void adicionar_chave(const int l, const int piece, const int casa){
    chaveAtual ^= hash[l][piece][casa];
    lockAtual ^= lock[l][piece][casa];
}