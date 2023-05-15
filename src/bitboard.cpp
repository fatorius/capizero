#include "bitboard.h"

#include "consts.h"
#include "update.h"

#include <string.h>
#include <stdint.h>

u64 mask[64];
u64 not_mask[64];

u64 bit_pieces[LADOS][TIPOS_DE_PIECES];
u64 bit_lados[LADOS];
u64 bit_total;
int tabuleiro[64];

int lado;
int xlado;
int cinquenta;
int roque;
int ply;
int hply;
int primeiro_lance[MAX_PLY];
int jogada;

void set_bit(u64 &bitboard, int bit){
    bitboard |= (UINT64_C(1) << bit);
}

void init_bits(){
    for (int x = 0; x < 64; x++){
        set_bit(mask[x], x);
        not_mask[x] = ~mask[x];
    }
}

int bitscan(u64 b){
    unsigned int folded;
    b ^= b - 1;
   folded = (int) b ^ (b >> 32);
   return lsb_64_table[folded * 0x78291ACF >> 26];
}

void init_board(){
    memset(bit_pieces, 0, sizeof(bit_pieces));
    memset(bit_lados, 0, sizeof(bit_lados));
    bit_total = 0;

    for (int x = 0; x < 64; ++x) {
        tabuleiro[x] = VAZIO;

        if(tabuleiro_inicial[x]<6){
            adicionar_piece(tabuleiro_inicial[x], cor_inicial[x], x);
        }
    }

    lado = 0;
    xlado = 1;
    cinquenta = 0;
    roque = 15;
    ply = 0;
    hply = 0;
    primeiro_lance[0] = 0;
    jogada = 0;
}