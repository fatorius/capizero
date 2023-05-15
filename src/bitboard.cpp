#include "bitboard.h"

#include "update.h"

#include <string.h>
#include <stdint.h>
#include <cstdlib>

u64 mask[CASAS_DO_TABULEIRO];
u64 not_mask[CASAS_DO_TABULEIRO];

u64 mask_cols[CASAS_DO_TABULEIRO];
u64 mask_isolados[CASAS_DO_TABULEIRO];

u64 mask_ala_do_rei;
u64 mask_ala_da_dama;

u64 mask_passados[LADOS][CASAS_DO_TABULEIRO];
u64 mask_path[LADOS][CASAS_DO_TABULEIRO];

u64 not_coluna_a;
u64 not_coluna_h;

u64 bit_pieces[LADOS][TIPOS_DE_PIECES];
u64 bit_lados[LADOS];
u64 bit_total;

int fileiras[LADOS][CASAS_DO_TABULEIRO];

int tabuleiro[CASAS_DO_TABULEIRO];

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
    for (int x = 0; x < CASAS_DO_TABULEIRO; x++){
        set_bit(mask[x], x);
        not_mask[x] = ~mask[x];

        for (int y = 0; y < CASAS_DO_TABULEIRO; y++){
            if (colunas[x] == colunas[y]){
                set_bit(mask_cols[x], y);

                if (linhas[x] < linhas[y]){
                    set_bit(mask_path[BRANCAS][x], y);
                }
                if (linhas[x] > linhas[y]){
                    set_bit(mask_path[PRETAS][x], y);
                }
            }

            if (abs(colunas[x] - colunas[y]) < 2){
                if (linhas[x] < linhas[y] && linhas[y] < COLUNA_H){
                    set_bit(mask_passados[BRANCAS][x], y);
                }
                if (linhas[x] > linhas[y] && linhas[x] > COLUNA_A){
                    set_bit(mask_passados[BRANCAS][x], y);
                }
            }

            if (abs(colunas[x] - colunas[y]) == 1){
                set_bit(mask_isolados[x], y);
            }
        }

        fileiras[BRANCAS][x] = linhas[x];
        fileiras[PRETAS][x] = 7 - linhas[x];

        if (colunas[x] < COLUNA_D){
            set_bit(mask_ala_da_dama, x);
        }

        if (colunas[x] > COLUNA_E){
            set_bit(mask_ala_do_rei, x);
        }
    }

    not_coluna_a = ~mask_cols[COLUNA_A];
    not_coluna_h = ~mask_cols[COLUNA_H];
}

void init_vetores(){

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

    for (int x = 0; x < CASAS_DO_TABULEIRO; ++x) {
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
