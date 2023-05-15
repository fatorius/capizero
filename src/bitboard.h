#ifndef BITBOARD
#define BITBOARD

#include <cstdint>

#include "consts.h"

typedef uint64_t u64;

extern u64 mask[CASAS_DO_TABULEIRO];
extern u64 not_mask[CASAS_DO_TABULEIRO];

void init_bits();
void init_board();

void set_bit(u64 &bitboard, int bit);

int bitscan(u64 b);

extern u64 bit_pieces[LADOS][TIPOS_DE_PIECES];
extern u64 bit_lados[LADOS];
extern u64 bit_total;
extern int tabuleiro[CASAS_DO_TABULEIRO];

extern int lado;
extern int xlado;
extern int cinquenta;
extern int roque;
extern int ply;
extern int hply;
extern int primeiro_lance[MAX_PLY];
extern int jogada;

#endif