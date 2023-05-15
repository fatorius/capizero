#include "update.h"

#include "bitboard.h"
#include "consts.h"

void adicionar_piece(const int lado, const int piece, const int casa){
    tabuleiro[casa] = piece;

    bit_lados[lado] |= mask[casa];
    bit_pieces[lado][piece] |= mask[casa];
    bit_total = bit_lados[BRANCAS] | bit_lados[PRETAS];
}