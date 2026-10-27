#ifndef GEN
#define GEN

#include <cstdint>

#include "bitboard.h"
#include "params.h"
#include "magics.h"

#ifdef USE_PEXT
#include <x86intrin.h>
#endif

struct Estado;

namespace Gen{
    typedef struct{
        uint8_t inicio;
        uint8_t destino;
        uint8_t promove;
        uint8_t _pad;
        int32_t score;
    } lance;

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

    extern Bitboard::u64 bit_casas_relevantes_bispo[CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_casas_relevantes_torres[CASAS_DO_TABULEIRO];

    extern Bitboard::u64 bit_peao_capturas[LADOS][CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_peao_defende[LADOS][CASAS_DO_TABULEIRO];

    inline Bitboard::u64 atacantes_bispo_occ(int casa, Bitboard::u64 occ){
        #ifdef USE_PEXT
            return bit_magicas_bispo[casa][_pext_u64(occ, bit_casas_relevantes_bispo[casa])];
        #else
            return bit_magicas_bispo[casa][((occ & bit_casas_relevantes_bispo[casa]) * Magics::magicas_bispos[casa]) >> (Magics::bits_indices_bispos[casa])];
        #endif
    }
    inline Bitboard::u64 atacantes_torre_occ(int casa, Bitboard::u64 occ){
        #ifdef USE_PEXT
            return bit_magicas_torre[casa][_pext_u64(occ, bit_casas_relevantes_torres[casa])];
        #else
            return bit_magicas_torre[casa][((occ & bit_casas_relevantes_torres[casa]) * Magics::magicas_torres[casa]) >> (Magics::bits_indices_torres[casa])];
        #endif
    }

    void init_lookup_tables();
    void gerar_lances(Estado &est, const int lado_a_mover, const int contraLado);
    void gerar_capturas_busca(Estado &est, const int lado_a_mover, const int contraLado);
    void gerar_silenciosos(Estado &est, const int lado_a_mover, const int contraLado);
    void gerar_capturas(Estado &est, const int lado_a_mover, const int contraLado);
    bool lance_pseudo_legal(Estado &est, const int inicio, const int destino, const int promove);
    unsigned long long perft_node(Estado &est, int profunidade);
    unsigned long long perft(Estado &est, int profunidade);
};

#endif