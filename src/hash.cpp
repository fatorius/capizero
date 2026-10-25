#include "hash.h"

#include <stdio.h>
#include <cstdlib>
#include <cstring>

#include "consts.h"
#include "values.h"
#include "params.h"

#include "gen.h"
#include "game.h"
#include "estado.h"

Bitboard::u64 hash[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
Bitboard::u64 lock[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];

size_t Hash::tt_entries_per_side = 0;

#define MATE_THRESHOLD (VALOR_XEQUE_MATE_BRANCAS - MAX_PLY)

static int score_to_tt(int s, int ply){
    if (s >=  MATE_THRESHOLD) return s + ply;
    if (s <= -MATE_THRESHOLD) return s - ply;
    return s;
}

static int score_from_tt(int s, int ply){
    if (s >=  MATE_THRESHOLD) return s - ply;
    if (s <= -MATE_THRESHOLD) return s + ply;
    return s;
}

Hash::hashp *hashpos[LADOS] = {NULL, NULL};

void Hash::liberar_memoria(){
    delete[] hashpos[BRANCAS];
    delete[] hashpos[PRETAS];
    hashpos[BRANCAS] = NULL;
    hashpos[PRETAS]  = NULL;
    tt_entries_per_side = 0;
}

void Hash::limpar_tt(){
    if (hashpos[BRANCAS] != NULL){
        std::memset(hashpos[BRANCAS], 0, sizeof(Hash::hashp) * tt_entries_per_side);
    }
    if (hashpos[PRETAS] != NULL){
        std::memset(hashpos[PRETAS], 0, sizeof(Hash::hashp) * tt_entries_per_side);
    }
}

void Hash::realocar_tt(int size_mb){
    if (size_mb < MIN_TT_MB) size_mb = MIN_TT_MB;
    if (size_mb > MAX_TT_MB) size_mb = MAX_TT_MB;

    // Split total budget across the two per-side tables.
    const size_t total_bytes = (size_t)size_mb * 1024u * 1024u;
    const size_t entries     = total_bytes / (2u * sizeof(Hash::hashp));

    delete[] hashpos[BRANCAS];
    delete[] hashpos[PRETAS];

    hashpos[BRANCAS] = new Hash::hashp[entries]();
    hashpos[PRETAS]  = new Hash::hashp[entries]();

    tt_entries_per_side = entries;
}

int aleatorio(const int x){
    return rand() % x;
}

static Bitboard::u64 aleatorio64(){
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

}

static void ensure_tt_allocated(){
    if (Hash::tt_entries_per_side == 0){
        Hash::realocar_tt(DEFAULT_TT_MB);
    }
}

void Hash::adicionar_chave(Estado &est, const int l, const int piece, const int casa){
    est.chaveAtual ^= hash[l][piece][casa];
    est.lockAtual ^= lock[l][piece][casa];
}

void Hash::adicionar_hash(const Estado &est, const int ld, const Gen::lance lc,
                          const int score, const int depth, const int bound,
                          const int promove){
    ensure_tt_allocated();
    hashp* ptr = &hashpos[ld][est.chaveAtual % tt_entries_per_side];

    if (ptr->hashlock == est.lockAtual && depth < ptr->depth){
        return;
    }

    ptr->hashlock = est.lockAtual;
    ptr->inicio   = (uint16_t) lc.inicio;
    ptr->dest     = (uint16_t) lc.destino;
    ptr->score    = (int32_t)  score_to_tt(score, est.ply);
    ptr->depth    = (int8_t)   depth;
    ptr->bound    = (uint8_t)  bound;
    ptr->promove  = (uint8_t)  promove;
}

void Hash::adicionar_pontuacao_de_hash(Estado &est){
    for (int lance = est.qntt_lances_totais[est.ply]; lance < est.qntt_lances_totais[est.ply + 1]; lance++){
        const Gen::lance &m = est.lista_de_lances[lance];
        if (m.inicio != est.hash_inicio || m.destino != est.hash_destino){
            continue;
        }
        if (m.promove != est.hash_promove){
            continue;
        }
        est.lista_de_lances[lance].score = PONTUACAO_HASH;
        return;
    }
}

Bitboard::u64 Hash::obter_lock(const Estado &est){
    Bitboard::u64 loc = 0;
    int l;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        l = 0;
        if (est.tabuleiro[casa] != VAZIO){
            if (est.bit_lados[PRETAS] & Bitboard::mask[casa]){
                l = 1;
            }

            loc ^= lock[l][est.tabuleiro[casa]][casa];
        }
    }

    return loc;
}

Bitboard::u64 Hash::obter_chave(const Estado &est){
    Bitboard::u64 chave = 0;
    int l;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        l = 0;
        if (est.tabuleiro[casa] != VAZIO){
            if (est.bit_lados[PRETAS] & Bitboard::mask[casa]){
                l = 1;
            }

            chave ^= hash[l][est.tabuleiro[casa]][casa];
        }
    }

    return chave;
}

bool Hash::hash_lookup(Estado &est, const int l){
    ensure_tt_allocated();
    const Bitboard::u64 idx = est.chaveAtual % tt_entries_per_side;
    const hashp &e = hashpos[l][idx];

    if (e.hashlock != est.lockAtual){
        return false;
    }

    est.hash_inicio  = e.inicio;
    est.hash_destino = e.dest;
    est.hash_score   = score_from_tt(e.score, est.ply);
    est.hash_depth   = e.depth;
    est.hash_bound   = e.bound;
    est.hash_promove = e.promove;

    return true;
}