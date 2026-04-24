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
    void gerar_lances(const int lado_a_mover, const int contraLado);
    void gerar_capturas(const int lado_a_mover, const int contraLado);
    unsigned long long perft_node(int profunidade);
    unsigned long long perft(int profunidade);
};

#endif