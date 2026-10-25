#ifndef ESTADO
#define ESTADO

#include "bitboard.h"
#include "game.h"
#include "gen.h"
#include "eval.h"

// Estado mutável de uma busca: tabuleiro, histórico do jogo, lista de lances,
// heurísticas de ordenação, chaves Zobrist e material. Cada thread de busca
// terá o seu; tabelas de lookup, tabelas de avaliação e a TT são compartilhadas.
// Toda função que lê ou altera a posição recebe o Estado por referência.
struct Estado{
    // Tabuleiro
    Bitboard::u64 bit_pieces[LADOS][TIPOS_DE_PIECES];
    Bitboard::u64 bit_lados[LADOS];
    Bitboard::u64 bit_total;
    int tabuleiro[CASAS_DO_TABULEIRO];

    // Jogo
    int lado;
    int xlado;
    int cinquenta;
    int roque;
    int ply;
    int hply;
    int jogada;
    int qntt_lances_totais[MAX_PLY];

    // Chaves Zobrist da posição e resultado do último Hash::hash_lookup
    Bitboard::u64 chaveAtual, lockAtual;
    int hash_inicio, hash_destino;
    int hash_score, hash_depth, hash_bound, hash_promove;

    // Material
    int peao_mat[LADOS];
    int piece_mat[LADOS];
    int fase_valor;

    // Busca e ordenação de lances
    int lances_avaliados;
    int mc; // próxima posição livre em lista_de_lances durante a geração
    Gen::lance killers_primarios[MAX_PLY];
    Gen::lance killers_secundarios[MAX_PLY];
    int historico_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
    Gen::lance contraLance_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];

    Game::jogo lista_do_jogo[PILHA_DO_JOGO];
    Gen::lance lista_de_lances[PILHA_DE_LANCES];

    // Hash de peões (~2,5 MB)
    Eval::EntradaPeoes hash_peoes[PAWN_HASH_SIZE];
    Eval::EntradaPeoes entrada_avulsa;
};

// Posição do jogo usada pelo UCI, bench, testes e ferramentas.
extern Estado estado_principal;

namespace Gen{
    inline Bitboard::u64 atacantes_bispo(const Estado &est, int casa){
        return atacantes_bispo_occ(casa, est.bit_total);
    }
    inline Bitboard::u64 atacantes_torre(const Estado &est, int casa){
        return atacantes_torre_occ(casa, est.bit_total);
    }
};

#endif
