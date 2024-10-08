#include "hash.h"

#include <stdio.h>
#include <cstdlib>

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

int aleatorio(const int x){
    return rand() % x;
}

void Hash::iniciar_hash(){
    int piece, casa;

    for (piece = 0; piece < TIPOS_DE_PIECES; piece++){
        for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
            hash[BRANCAS][piece][casa] = aleatorio(HASHSIZE);
            hash[PRETAS][piece][casa] = aleatorio(HASHSIZE);
            lock[BRANCAS][piece][casa] = aleatorio(HASHSIZE);
            lock[PRETAS][piece][casa] = aleatorio(HASHSIZE);
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
    hashp* ptr = &hashpos[ld][chaveAtual];

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
    if (hashpos[l][chaveAtual].hashlock != lockAtual){
        return false;
    }

    hash_inicio = hashpos[l][chaveAtual].inicio;
    hash_destino = hashpos[l][chaveAtual].dest;

    return true;
}