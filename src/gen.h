#ifndef GEN
#define GEN

#include <cstdint>

#include "bitboard.h"
#include "params.h"

namespace Gen{
    // Packed move: squares fit in 6 bits (0-63), promotion piece in 4 bits
    // (0-15, actual range 0-5). Stored as uint8_t for direct byte load/store
    // (no bit-field shift+mask overhead). Score stays int32_t — values reach
    // ~1e8 for the TT move boost, well outside 16-bit range.
    // Layout: 4 bytes payload + 4 bytes score = 8 bytes per entry
    // (was 16 bytes as 4× int). Halves footprint of lista_de_lances,
    // killers_primarios/secundarios, and contraLance_heuristica.
    typedef struct{
        uint8_t inicio;
        uint8_t destino;
        uint8_t promove;
        uint8_t _pad;
        int32_t score;
    } lance;

    extern lance lista_de_lances[PILHA_DE_LANCES];

    extern Bitboard::u64 bit_esquerda[LADOS][CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_direita[LADOS][CASAS_DO_TABULEIRO];

    extern int peao_direita[LADOS][CASAS_DO_TABULEIRO];
    extern int peao_esquerda[LADOS][CASAS_DO_TABULEIRO];

    extern Bitboard::u64 bit_moves_peao[LADOS][CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_moves_cavalo[CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_moves_bispo[CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_moves_torre[CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_moves_dama[CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_moves_rei[CASAS_DO_TABULEIRO];

    extern Bitboard::u64 bit_magicas_bispo[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];
    extern Bitboard::u64 bit_magicas_torre[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];

    extern Bitboard::u64 bit_peao_capturas[LADOS][CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_peao_defende[LADOS][CASAS_DO_TABULEIRO];

    void init_lookup_tables();
    // Full pseudo-legal move list (captures + quiets). Used by perft and by
    // anything outside `Search::pesquisa` that wants the whole list at once.
    void gerar_lances(const int lado_a_mover, const int contraLado);
    // Captures only (including EP and pawn capture-promotions with Q+N
    // variants). Emits Q+N for capture-promos; contrast with `gerar_capturas`
    // below which emits Q-only for qsearch. Used as the first stage of
    // `pesquisa`'s lazy move generation.
    void gerar_capturas_busca(const int lado_a_mover, const int contraLado);
    // Quiet moves only (castling, pawn pushes including quiet promos, and all
    // non-capture piece moves). Appends onto an existing capture list by
    // starting from `qntt_lances_totais[ply+1]`; must run after a capture
    // generator that set that boundary.
    void gerar_silenciosos(const int lado_a_mover, const int contraLado);
    // Qsearch captures (Q-only promo, no EP). Do not confuse with
    // `gerar_capturas_busca`.
    void gerar_capturas(const int lado_a_mover, const int contraLado);
    unsigned long long perft_node(int profunidade);
    unsigned long long perft(int profunidade);
};

#endif