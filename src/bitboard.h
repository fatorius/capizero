#ifndef BITBOARD
#define BITBOARD

#include <cstdint>

#include "consts.h"

typedef uint64_t u64;

extern u64 mask[CASAS_DO_TABULEIRO];
extern u64 not_mask[CASAS_DO_TABULEIRO];
extern u64 mask_isolados[CASAS_DO_TABULEIRO];
extern u64 mask_cols[CASAS_DO_TABULEIRO];
extern u64 mask_rows[CASAS_DO_TABULEIRO];

extern u64 mask_passados[LADOS][CASAS_DO_TABULEIRO];
extern u64 mask_path[LADOS][CASAS_DO_TABULEIRO];

extern u64 bit_entre[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
extern u64 vetor_bits[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
extern u64 mask_vetores[CASAS_DO_TABULEIRO][8];

extern u64 bit_pieces[LADOS][TIPOS_DE_PIECES];
extern u64 bit_lados[LADOS];
extern u64 bit_total;

extern u64 bit_casas_relevantes_bispo[CASAS_DO_TABULEIRO];
extern u64 bit_casas_relevantes_torres[CASAS_DO_TABULEIRO];

extern u64 not_coluna_a;
extern u64 not_coluna_h;

extern u64 bordas;
extern u64 bordas_neg;

extern u64 mask_ala_do_rei;
extern u64 mask_ala_da_dama;

extern int fileiras[LADOS][CASAS_DO_TABULEIRO];

extern int tabuleiro[CASAS_DO_TABULEIRO];

void init_bits();
void init_board();
void init_vetores();

void set_bit(u64 &bitboard, int bit);

int obter_borda(int casa, int soma);

int popcount(u64 b);
int bitscan(u64 b);

#endif