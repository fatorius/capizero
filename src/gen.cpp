#include "gen.h"

u64 bit_esquerda[LADOS][CASAS_DO_TABULEIRO];
u64 bit_direita[LADOS][CASAS_DO_TABULEIRO];

u64 bit_peao_capturas[LADOS][CASAS_DO_TABULEIRO];
u64 bit_peao_defende[LADOS][CASAS_DO_TABULEIRO];

u64 peao_direita[LADOS][CASAS_DO_TABULEIRO];
u64 peao_esquerda[LADOS][CASAS_DO_TABULEIRO];

u64 peao_mais[LADOS][CASAS_DO_TABULEIRO];
u64 peao_double[LADOS][CASAS_DO_TABULEIRO];

u64 bit_moves_cavalo[CASAS_DO_TABULEIRO];
u64 bit_moves_bispo[CASAS_DO_TABULEIRO];
u64 bit_moves_torre[CASAS_DO_TABULEIRO];
u64 bit_moves_dama[CASAS_DO_TABULEIRO];
u64 bit_moves_rei[CASAS_DO_TABULEIRO];

int dtb_moves[CASAS_DO_TABULEIRO][9];
int cavalo_moves[CASAS_DO_TABULEIRO][9];
int rei_moves[CASAS_DO_TABULEIRO][9];

void init_peao_lookups(){
    for (int x = 0; x < 64; x++){
        peao_esquerda[BRANCAS][x] = -1;
        peao_esquerda[PRETAS][x] = -1;
        peao_direita[BRANCAS][x] = -1;
        peao_direita[PRETAS][x] = -1;
    
        if (colunas[x] > COLUNA_A){
            if (linhas[x] < COLUNA_H){
                peao_esquerda[BRANCAS][x] = x + 7;
                set_bit(bit_peao_capturas[BRANCAS][x], peao_esquerda[BRANCAS][x]);
                set_bit(bit_esquerda[BRANCAS][x], peao_esquerda[BRANCAS][x]);
            }
            if (linhas[x] > COLUNA_A){
                peao_esquerda[PRETAS][x] = x - 9;
                set_bit(bit_peao_capturas[PRETAS][x], peao_esquerda[PRETAS][x]);
                set_bit(bit_esquerda[PRETAS][x], peao_esquerda[PRETAS][x]);
            }
        }

        if (colunas[x] < COLUNA_H){
            if (linhas[x] < COLUNA_H){
                peao_direita[BRANCAS][x] = x + 9;
                set_bit(bit_peao_capturas[BRANCAS][x], peao_esquerda[BRANCAS][x]);
                set_bit(bit_esquerda[BRANCAS][x], peao_esquerda[BRANCAS][x]);
            }
            if (linhas[x] > COLUNA_A){
                peao_direita[PRETAS][x] = x - 7;
                set_bit(bit_peao_capturas[PRETAS][x], peao_direita[PRETAS][x]);
                set_bit(bit_direita[PRETAS][x], peao_direita[PRETAS][x]);
            }
        }

        bit_peao_defende[BRANCAS][x] = bit_peao_capturas[PRETAS][x];
        bit_peao_defende[PRETAS][x] = bit_peao_capturas[BRANCAS][x];

        if (linhas[x] < COLUNA_H){
            peao_mais[BRANCAS][x] = x + 8;
        }
        if (linhas[x] < COLUNA_G){
            peao_double[BRANCAS][x] = x + 16;
        }
        if (linhas[x] > COLUNA_A){ 
            peao_mais[PRETAS][x] = x - 8;
        }
        if (linhas[x] > COLUNA_B){
            peao_double[PRETAS][x] = x - 16;
        }

    }
}

void init_cavalo_lookups(){
    int k, x = 0;
    
    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        k = 0;

        if (linhas[x] < 6 && colunas[x] < COLUNA_H){
            cavalo_moves[x][k++] = x + 17;
        }
        if (linhas[x] < COLUNA_H && colunas[x] < COLUNA_G){
            cavalo_moves[x][k++] = x + 10;
        }
        if (linhas[x] < 6 && colunas[x] > COLUNA_A){
            cavalo_moves[x][k++] = x + 15;
        }
        if (linhas[x] < COLUNA_H && colunas[x] > COLUNA_B){
            cavalo_moves[x][k++] = x + 6;
        }
        if (linhas[x] > 1 && colunas[x] < COLUNA_H){
            cavalo_moves[x][k++] = x - 15;
        }
        if (linhas[x] > COLUNA_A && colunas[x] < COLUNA_G){
            cavalo_moves[x][k++] = x - 6;
        }
        if (linhas[x] > 1 && colunas[x] > COLUNA_A){
            cavalo_moves[x][k++] = x - 17;
        }
        if (linhas[x] > COLUNA_A && colunas[x] > COLUNA_B){
            cavalo_moves[x][k++] = x - 10;
        }

        cavalo_moves[x][k] = -1;
    }

    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        k = 0;
        while (cavalo_moves[x][k] > -1){
            bit_moves_cavalo[x] |= mask[cavalo_moves[x][k]];
            k++;
        }
    }
}

void init_rei_lookups(){
    int x, y, k = 0;

    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        k = 0;
        y = 0;

        if (colunas[x] > COLUNA_A){
            rei_moves[x][y++] = x-1;    
        }
        if (colunas[x] < COLUNA_H){
            rei_moves[x][y++] = x+1;    
        }
        if (linhas[x] > COLUNA_A){
            rei_moves[x][y++] = x-8;    
        }
        if (linhas[x] < COLUNA_H){
            rei_moves[x][y++] = x+8;    
        }
        if (colunas[x] < COLUNA_H && linhas[x] < COLUNA_H){
            rei_moves[x][y++] = x+9;    
        }
        if (colunas[x] > COLUNA_A && linhas[x] < COLUNA_H){
            rei_moves[x][y++] = x+7;    
        }
        if (colunas[x] > COLUNA_A && linhas[x] > COLUNA_A){
            rei_moves[x][y++] = x-9;    
        }
        if (colunas[x] < COLUNA_H && linhas[x] > COLUNA_A){
            rei_moves[x][y++] = x-7;    
        }

        rei_moves[x][y] = -1;
    }

    k = 0;
    while (rei_moves[x][k] > -1){
        bit_moves_rei[x] |= mask[rei_moves[x][k]];
        k++;
    }
}

void init_dtb_lookups(){
    int x, y = 0;

    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        y=0;

        for (int z = 0; z < 8; z++){
            dtb_moves[x][z] = -1;
        }

        if (colunas[x] > COLUNA_A){
            dtb_moves[x][OESTE] = x-1;
            rei_moves[x][y++] = x-1;    
        }
        if (colunas[x] < COLUNA_H){
            dtb_moves[x][LESTE] = x+1;
            rei_moves[x][y++] = x+1;    
        }
        if (linhas[x] > COLUNA_A){
            dtb_moves[x][SUL] = x-8;
            rei_moves[x][y++] = x-8;    
        }
        if (linhas[x] < COLUNA_H){
            dtb_moves[x][NORTE] = x+8;
            rei_moves[x][y++] = x+8;    
        }
        if (colunas[x] < COLUNA_H && linhas[x] < COLUNA_H){
            dtb_moves[x][NE] = x+9;
            rei_moves[x][y++] = x+9;    
        }
        if (colunas[x] > COLUNA_A && linhas[x] < COLUNA_H){
            dtb_moves[x][NO] = x+7;
            rei_moves[x][y++] = x+7;    
        }
        if (colunas[x] > COLUNA_A && linhas[x] > COLUNA_A){
            dtb_moves[x][SO] = x-9;
            rei_moves[x][y++] = x-9;    
        }
        if (colunas[x] < COLUNA_H && linhas[x] > COLUNA_A){
            dtb_moves[x][SE] = x-7;
            rei_moves[x][y++] = x-7;    
        }

        rei_moves[x][y] = -1;
    }

    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        bit_moves_bispo[x] = 0;
        bit_moves_torre[x] = 0;
        bit_moves_dama[x] = 0;

        for (y = 0; y < CASAS_DO_TABULEIRO; y++){            
            if (x == y){
                continue;
            }

            if (no_diag[x] == no_diag[y] || ne_diag[x] == ne_diag[y]){
                bit_moves_bispo[x] |= mask[y];
                bit_moves_dama[x] |= mask[y];
            }

            if (colunas[x] == colunas[y] || linhas[x] == linhas[y]){
                bit_moves_torre[x] |= mask[y];
                bit_moves_dama[x] |= mask[y];
            }
        }
    }
}

void init_lookup_tables(){
    init_peao_lookups();
    init_cavalo_lookups();
    init_rei_lookups();
    init_dtb_lookups();
}