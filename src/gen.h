#ifndef GEN
#define GEN

#include "bitboard.h"

extern u64 bit_moves_peao[LADOS][CASAS_DO_TABULEIRO];
extern u64 bit_moves_cavalo[CASAS_DO_TABULEIRO];
extern u64 bit_moves_bispo[CASAS_DO_TABULEIRO];
extern u64 bit_moves_torre[CASAS_DO_TABULEIRO];
extern u64 bit_moves_dama[CASAS_DO_TABULEIRO];
extern u64 bit_moves_rei[CASAS_DO_TABULEIRO];

void init_lookup_tables();
void gerar_lances(const int lado, const int contraLado);

#endif