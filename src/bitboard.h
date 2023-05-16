#ifndef BITBOARD
#define BITBOARD

#include <cstdint>

#include "consts.h"

typedef uint64_t u64;

extern u64 mask[CASAS_DO_TABULEIRO];
extern u64 not_mask[CASAS_DO_TABULEIRO];
extern u64 mask_isolados[CASAS_DO_TABULEIRO];

extern u64 mask_passados[LADOS][CASAS_DO_TABULEIRO];
extern u64 mask_path[LADOS][CASAS_DO_TABULEIRO];

extern u64 bit_entre[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
extern u64 vetor_bits[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
extern u64 mask_vetores[CASAS_DO_TABULEIRO][8];

extern u64 bit_pieces[LADOS][TIPOS_DE_PIECES];
extern u64 bit_lados[LADOS];
extern u64 bit_total;

extern int fileiras[LADOS][CASAS_DO_TABULEIRO];

extern int tabuleiro[CASAS_DO_TABULEIRO];

extern int lado;
extern int xlado;
extern int cinquenta;
extern int roque;
extern int ply;
extern int hply;
extern int primeiro_lance[MAX_PLY];
extern int jogada;

void init_bits();
void init_board();
void init_vetores();

void set_bit(u64 &bitboard, int bit);

int obter_borda(int casa, int soma);
int bitscan(u64 b);


#endif