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

u64 bit_entre[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
u64 vetor_bits[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
u64 mask_vetores[CASAS_DO_TABULEIRO][8];

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
    int x, y;
    int z;

    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        for (y = 0; x < CASAS_DO_TABULEIRO; y++){
            if (linhas[x] == linhas[x]){
                if (y > x){
                    for (z = x + 1; z < y; z++){
                        set_bit(bit_entre[x][y], z);
                    }
                    for (z = x + 1; z <= y; z++){
                        set_bit(vetor_bits[x][y], z);
                    }
                }
                else{
                    for (z = y + 1; z < x; z++){
                        set_bit(bit_entre[x][y], z);
                    }
                    for (z = y; z < x; z++){
                        set_bit(vetor_bits[x][y], z);
                    }
                }
            }

            if (colunas[x] == colunas[y]){
                if (y > x){
                    for (z = x + 8; z < y; z += 8){
                        set_bit(bit_entre[x][y], z);
                    }
                    for (z = x + 8; z <= y; z += 8){
                        set_bit(vetor_bits[x][y], z);
                    }
                }
                else{
                    for (z = y + 8; z < x; z += 8){
                        set_bit(bit_entre[x][y], z);
                    }
                    for (z = y; z < x; z += 8){
                        set_bit(vetor_bits[x][y], z);
                    }
                }
            }

            if (no_diag[x] == no_diag[y]){
                if (y > x){
                    for (z = x + 7; z < y; x += 7){
                        set_bit(bit_entre[x][y], z);
                    }
                    for (z = x + 7; z <= y; z += 7){
                        set_bit(vetor_bits[x][y], z);
                    }
                }
                else{
                    for (z = y + 7; z < x; z += 7){
                        set_bit(bit_entre[x][y], z);
                    }
                    for (z = y; z < x; z += 7){
                        set_bit(vetor_bits[x][y], z);
                    }
                }
            }

            if (ne_diag[x] == ne_diag[y]){
                if (y > x){
                    for (z = x + 9; z < y; z += 9){
                        set_bit(bit_entre[x][y], z);
                    }
                    for (z = x + 9; z <= y; z += 9){
                        set_bit(vetor_bits[x][y], z);
                    }
                }
                else{
                    for (z = y + 9; z < x; z += 9){
                        set_bit(bit_entre[x][y], z);
                    }
                    for (z = y; z < x; z += 9){
                        set_bit(vetor_bits[x][y], z);
                    }
                }
            }
        }
    }

    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        mask_vetores[x][NORTE] = vetor_bits[x][56+colunas[x]];
        mask_vetores[x][SUL] = vetor_bits[x][colunas[x]];
        mask_vetores[x][OESTE] = vetor_bits[x][linhas[x] * 8];
        mask_vetores[x][LESTE] = vetor_bits[x][(linhas[x] * 8) + 7];

        if (colunas[x] > COLUNA_A && linhas[x] < COLUNA_H){
            mask_vetores[x][NO] = vetor_bits[x][obter_borda(x, 7)];
        }
        if (colunas[x] < COLUNA_H && linhas[x] < COLUNA_H){
            mask_vetores[x][NE] = vetor_bits[x][obter_borda(x, 9)];
        }
        if (linhas[x] > COLUNA_A && colunas[x] > COLUNA_A){
            mask_vetores[x][SO] = vetor_bits[x][obter_borda(x, -9)];
        }
        if (linhas[x] > COLUNA_A && colunas[x] < COLUNA_H){
            mask_vetores[x][SE] = vetor_bits[x][obter_borda(x, -7)];
        }
    }
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

void set_bit(u64 &bitboard, int bit){
    bitboard |= (UINT64_C(1) << bit);
}

int obter_borda(int casa, int soma){
    do {
        casa += soma;
    }
    while (colunas[casa] > COLUNA_A && colunas[casa] < COLUNA_H && linhas[casa] > COLUNA_A && linhas[casa] < COLUNA_H);

    return casa;
}

int bitscan(u64 b){
    unsigned int folded;
    b ^= b - 1;
    folded = (int) b ^ (b >> 32);
    return lsb_64_table[folded * 0x78291ACF >> 26];
}
