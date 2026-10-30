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

// Direitos de roque (índice = est.roque, 0 sem direitos = 0) e coluna de en passant.
static Bitboard::u64 hash_roque[16], lock_roque[16];
static Bitboard::u64 hash_ep[8], lock_ep[8];

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

static_assert(sizeof(Hash::hashp) == 16, "entrada da TT deve ter 16 bytes");

Bitboard::u64 Hash::empacotar(const EntradaTT &e){
    return  (Bitboard::u64)(uint32_t)e.score
         | ((Bitboard::u64)(e.inicio  & 63)              << 32)
         | ((Bitboard::u64)(e.destino & 63)              << 38)
         | ((Bitboard::u64)(uint8_t)(int8_t)e.depth      << 44)
         | ((Bitboard::u64)(e.bound   & 3)               << 52)
         | ((Bitboard::u64)(e.promove & 7)               << 54);
}

Hash::EntradaTT Hash::desempacotar(Bitboard::u64 dados){
    EntradaTT e;
    e.score   = (int32_t)(uint32_t)(dados & 0xFFFFFFFFULL);
    e.inicio  = (int)((dados >> 32) & 63);
    e.destino = (int)((dados >> 38) & 63);
    e.depth   = (int8_t)(uint8_t)((dados >> 44) & 0xFF);
    e.bound   = (int)((dados >> 52) & 3);
    e.promove = (int)((dados >> 54) & 7);
    return e;
}

static void zerar_tabela(Hash::hashp *tabela, size_t n){
    for (size_t i = 0; i < n; i++){
        tabela[i].chave_xor.store(0, std::memory_order_relaxed);
        tabela[i].dados.store(0, std::memory_order_relaxed);
    }
}

void Hash::liberar_memoria(){
    delete[] hashpos[BRANCAS];
    delete[] hashpos[PRETAS];
    hashpos[BRANCAS] = NULL;
    hashpos[PRETAS]  = NULL;
    tt_entries_per_side = 0;
}

void Hash::limpar_tt(){
    if (hashpos[BRANCAS] != NULL){
        zerar_tabela(hashpos[BRANCAS], tt_entries_per_side);
    }
    if (hashpos[PRETAS] != NULL){
        zerar_tabela(hashpos[PRETAS], tt_entries_per_side);
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

    hashpos[BRANCAS] = new Hash::hashp[entries];
    hashpos[PRETAS]  = new Hash::hashp[entries];

    tt_entries_per_side = entries;
    limpar_tt();
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

    // Depois das peças, para que as chaves das peças continuem as mesmas.
    for (int r = 1; r < 16; r++){
        hash_roque[r] = aleatorio64();
        lock_roque[r] = aleatorio64();
    }
    for (int c = 0; c < 8; c++){
        hash_ep[c] = aleatorio64();
        lock_ep[c] = aleatorio64();
    }
}

void Hash::garantir_tt(){
    if (Hash::tt_entries_per_side == 0){
        Hash::realocar_tt(DEFAULT_TT_MB);
    }
}

void Hash::adicionar_chave(Estado &est, const int l, const int piece, const int casa){
    est.chaveAtual ^= hash[l][piece][casa];
    est.lockAtual ^= lock[l][piece][casa];
}

int Hash::coluna_en_passant(const Estado &est){
    if (est.hply < 1){
        return -1;
    }

    const Game::jogo &ultimo = est.lista_do_jogo[est.hply - 1];
    const int ep = ultimo.destino;
    if (est.tabuleiro[ep] != P || abs(ultimo.inicio - ep) != AVANCO_DUPLO){
        return -1;
    }

    const int coluna = Consts::colunas[ep];
    if ((coluna > COLUNA_A && (Bitboard::mask[ep-1] & est.bit_pieces[est.lado][P])) ||
        (coluna < COLUNA_H && (Bitboard::mask[ep+1] & est.bit_pieces[est.lado][P]))){
        return coluna;
    }
    return -1;
}

void Hash::atualizar_roque_ep(Estado &est, const int roque_antes, const int ep_antes, const int ep){
    if (roque_antes != est.roque){
        est.chaveAtual ^= hash_roque[roque_antes] ^ hash_roque[est.roque];
        est.lockAtual  ^= lock_roque[roque_antes] ^ lock_roque[est.roque];
    }

    if (ep_antes >= 0){
        est.chaveAtual ^= hash_ep[ep_antes];
        est.lockAtual  ^= lock_ep[ep_antes];
    }

    if (ep >= 0){
        est.chaveAtual ^= hash_ep[ep];
        est.lockAtual  ^= lock_ep[ep];
    }
}

void Hash::adicionar_hash(const Estado &est, const int ld, const Gen::lance lc,
                          const int score, const int depth, const int bound,
                          const int promove){
    Hash::garantir_tt();
    hashp* ptr = &hashpos[ld][est.chaveAtual % tt_entries_per_side];

    const Bitboard::u64 dados_atuais = ptr->dados.load(std::memory_order_relaxed);
    const Bitboard::u64 chave_atual  = ptr->chave_xor.load(std::memory_order_relaxed);

    if ((chave_atual ^ dados_atuais) == est.lockAtual && depth < desempacotar(dados_atuais).depth){
        return;
    }

    EntradaTT e;
    e.inicio  = lc.inicio;
    e.destino = lc.destino;
    e.score   = score_to_tt(score, est.ply);
    e.depth   = depth;
    e.bound   = bound;
    e.promove = promove;

    const Bitboard::u64 dados = empacotar(e);
    ptr->dados.store(dados, std::memory_order_relaxed);
    ptr->chave_xor.store(est.lockAtual ^ dados, std::memory_order_relaxed);
}

Hash::hashp *Hash::entrada(const Estado &est, const int l){
    Hash::garantir_tt();
    return &hashpos[l][est.chaveAtual % tt_entries_per_side];
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

    loc ^= lock_roque[est.roque];
    const int ep = coluna_en_passant(est);
    if (ep >= 0){
        loc ^= lock_ep[ep];
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

    chave ^= hash_roque[est.roque];
    const int ep = coluna_en_passant(est);
    if (ep >= 0){
        chave ^= hash_ep[ep];
    }

    return chave;
}

bool Hash::hash_lookup(Estado &est, const int l){
    Hash::garantir_tt();
    const hashp &h = hashpos[l][est.chaveAtual % tt_entries_per_side];

    // Cópias locais: a verificação e a decodificação usam o mesmo par de palavras.
    const Bitboard::u64 dados = h.dados.load(std::memory_order_relaxed);
    const Bitboard::u64 chave = h.chave_xor.load(std::memory_order_relaxed);

    if ((chave ^ dados) != est.lockAtual){
        return false;
    }

    const EntradaTT e = desempacotar(dados);

    est.hash_inicio  = e.inicio;
    est.hash_destino = e.destino;
    est.hash_score   = score_from_tt(e.score, est.ply);
    est.hash_depth   = e.depth;
    est.hash_bound   = e.bound;
    est.hash_promove = e.promove;

    return true;
}