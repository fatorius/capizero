#include "hash.h"

#include <stdio.h>
#include <cstdlib>

#include "consts.h"

#include "gen.h"
#include "game.h"

u64 hash[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
u64 lock[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];

u64 chaveAtual, lockAtual;

uint64_t colisoes;

hashp *hashpos[LADOS];

int hash_inicio, hash_destino;

void liberar_memoria(){
    delete hashpos[BRANCAS];
    delete hashpos[PRETAS];
}

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

void adicionar_hash(const int ld, const lance lc){
    hashp* ptr = &hashpos[ld][chaveAtual];

    ptr->hashlock = lockAtual;
    ptr->inicio = lc.inicio;
    ptr->dest = lc.destino;
}

void adicionar_pontuacao_de_hash(){
    for (int lance = qntt_lances_totais[ply]; lance < qntt_lances_totais[ply + 1]; lance++){
        if (lista_de_lances[lance].inicio == hash_inicio && lista_de_lances[lance].destino == hash_destino){
            lista_de_lances[lance].score = PONTUACAO_HASH;
            return;
        }
    }
}

u64 obter_lock(){
    u64 loc = 0;
    int l;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        l = 0;
        if (tabuleiro[casa] != VAZIO){
            if (bit_lados[PRETAS] & mask[casa]){
                l = 1;
            }

            loc ^= lock[l][tabuleiro[casa]][casa];
        }
    }

    return loc;
}

u64 obter_chave(){
    u64 chave = 0;
    int l;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        l = 0;
        if (tabuleiro[casa] != VAZIO){
            if (bit_lados[PRETAS] & mask[casa]){
                l = 1;
            }

            chave ^= hash[l][tabuleiro[casa]][casa];
        }
    }

    return chave;
}

bool hash_lookup(const int l){
    if (hashpos[l][chaveAtual].hashlock != lockAtual){
        return false;
    }

    hash_inicio = hashpos[l][chaveAtual].inicio;
    hash_destino = hashpos[l][chaveAtual].dest;

    return true;
}