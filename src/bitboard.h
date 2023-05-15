#ifndef BITBOARD
#define BITBOARD

#include <cstdint>

#include "params.h"

typedef uint64_t u64;

extern u64 bit_pieces[LADOS][TIPOS_DE_PIECES];
extern u64 bit_lados[LADOS];
extern u64 bit_total;

#endif