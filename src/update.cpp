#include "update.h"

#include <cstdlib>

#include "bitboard.h"
#include "consts.h"
#include "hash.h"
#include "game.h"
#include "attacks.h"

void adicionar_piece(const int l, const int piece, const int casa){
    tabuleiro[casa] = piece;

    adicionar_chave(l, piece, casa);

    bit_lados[l] |= mask[casa];
    bit_pieces[l][piece] |= mask[casa];
    bit_total = bit_lados[BRANCAS] | bit_lados[PRETAS];
}