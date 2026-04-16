#include "hash.h"

#include <stdio.h>
#include <cstdlib>
#include <cstring>

#include "consts.h"

#include "gen.h"
#include "game.h"

Bitboard::u64 hash[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
Bitboard::u64 lock[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];

Bitboard::u64 Hash::chaveAtual, Hash::lockAtual;
int Hash::hash_inicio, Hash::hash_destino;

Hash::hashp *hashpos[LADOS];

void Hash::liberar_memoria(){
    delete hashpos[BRANCAS];
    delete hashpos[PRETAS];
}

void Hash::limpar_tt(){
    if (hashpos[BRANCAS] != NULL){
        std::memset(hashpos[BRANCAS], 0, sizeof(Hash::hashp) * MAXHASH);
    }
    if (hashpos[PRETAS] != NULL){
        std::memset(hashpos[PRETAS], 0, sizeof(Hash::hashp) * MAXHASH);
    }
}

int aleatorio(const int x){
    // rand() intentionally unseeded: deterministic Zobrist keys across runs
    // give reproducible bench node counts and TT behavior for SPRT testing.
    // Do not call srand() here.
    return rand() % x;
}

static Bitboard::u64 aleatorio64(){
    // Build a full 64-bit value from four 16-bit chunks of rand().
    // RAND_MAX is guaranteed to be >= 32767, so 16-bit chunks are portable.
    return ((Bitboard::u64)(rand() & 0xFFFF))
         | ((Bitboard::u64)(rand() & 0xFFFF) << 16)
         | ((Bitboard::u64)(rand() & 0xFFFF) << 32)
         | ((Bitboard::u64)(rand() & 0xFFFF) << 48);
}

void Hash::iniciar_hash(){
    int piece, casa;

    for (piece = 0; piece < TIPOS_DE_PIECES; piece++){
        for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
            hash[BRANCAS][piece][casa] = aleatorio64();
            hash[PRETAS][piece][casa] = aleatorio64();
            lock[BRANCAS][piece][casa] = aleatorio64();
            lock[PRETAS][piece][casa] = aleatorio64();
        }
    }

    hashpos[BRANCAS] = new Hash::hashp[MAXHASH];
    hashpos[PRETAS] = new Hash::hashp[MAXHASH];
}

void Hash::adicionar_chave(const int l, const int piece, const int casa){
    chaveAtual ^= hash[l][piece][casa];
    lockAtual ^= lock[l][piece][casa];
}

void Hash::adicionar_hash(const int ld, const Gen::lance lc){
    hashp* ptr = &hashpos[ld][chaveAtual % MAXHASH];

    ptr->hashlock = lockAtual;
    ptr->inicio = lc.inicio;
    ptr->dest = lc.destino;
}

void Hash::adicionar_pontuacao_de_hash(){
    for (int lance = Game::qntt_lances_totais[Game::ply]; lance < Game::qntt_lances_totais[Game::ply + 1]; lance++){
        if (Gen::lista_de_lances[lance].inicio == hash_inicio && Gen::lista_de_lances[lance].destino == hash_destino){
            Gen::lista_de_lances[lance].score = PONTUACAO_HASH;
            return;
        }
    }
}

Bitboard::u64 Hash::obter_lock(){
    Bitboard::u64 loc = 0;
    int l;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        l = 0;
        if (Bitboard::tabuleiro[casa] != VAZIO){
            if (Bitboard::bit_lados[PRETAS] & Bitboard::mask[casa]){
                l = 1;
            }

            loc ^= lock[l][Bitboard::tabuleiro[casa]][casa];
        }
    }

    return loc;
}

Bitboard::u64 Hash::obter_chave(){
    Bitboard::u64 chave = 0;
    int l;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        l = 0;
        if (Bitboard::tabuleiro[casa] != VAZIO){
            if (Bitboard::bit_lados[PRETAS] & Bitboard::mask[casa]){
                l = 1;
            }

            chave ^= hash[l][Bitboard::tabuleiro[casa]][casa];
        }
    }

    return chave;
}

bool Hash::hash_lookup(const int l){
    const Bitboard::u64 idx = chaveAtual % MAXHASH;

    if (hashpos[l][idx].hashlock != lockAtual){
        return false;
    }

    hash_inicio = hashpos[l][idx].inicio;
    hash_destino = hashpos[l][idx].dest;

    return true;
}