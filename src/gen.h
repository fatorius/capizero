#ifndef GEN
#define GEN

#include <cstdint>

#include "bitboard.h"
#include "params.h"
#include "magics.h"

#ifdef USE_PEXT
#include <x86intrin.h>
#endif

namespace Gen{
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

    extern Bitboard::u64 bit_casas_relevantes_bispo[CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_casas_relevantes_torres[CASAS_DO_TABULEIRO];

    extern Bitboard::u64 bit_peao_capturas[LADOS][CASAS_DO_TABULEIRO];
    extern Bitboard::u64 bit_peao_defende[LADOS][CASAS_DO_TABULEIRO];

    inline Bitboard::u64 atacantes_bispo(int casa){
        #ifdef USE_PEXT
            return bit_magicas_bispo[casa][_pext_u64(Bitboard::bit_total, bit_casas_relevantes_bispo[casa])];
        #else
            return bit_magicas_bispo[casa][((Bitboard::bit_total & bit_casas_relevantes_bispo[casa]) * Magics::magicas_bispos[casa]) >> (Magics::bits_indices_bispos[casa])];
        #endif
    }
    inline Bitboard::u64 atacantes_torre(int casa){
        #ifdef USE_PEXT
            return bit_magicas_torre[casa][_pext_u64(Bitboard::bit_total, bit_casas_relevantes_torres[casa])];
        #else
            return bit_magicas_torre[casa][((Bitboard::bit_total & bit_casas_relevantes_torres[casa]) * Magics::magicas_torres[casa]) >> (Magics::bits_indices_torres[casa])];
        #endif
    }
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
    void gerar_lances(const int lado_a_mover, const int contraLado);
    void gerar_capturas_busca(const int lado_a_mover, const int contraLado);
    void gerar_silenciosos(const int lado_a_mover, const int contraLado);
    void gerar_capturas(const int lado_a_mover, const int contraLado);
    unsigned long long perft_node(int profunidade);
    unsigned long long perft(int profunidade);
};

#endif