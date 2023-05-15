#include "gen.h"

#include <string.h>

u64 bit_moves_peao[LADOS][CASAS_DO_TABULEIRO];
u64 bit_moves_cavalo[CASAS_DO_TABULEIRO];
u64 bit_moves_bispo[CASAS_DO_TABULEIRO];
u64 bit_moves_torre[CASAS_DO_TABULEIRO];
u64 bit_moves_dama[CASAS_DO_TABULEIRO];
u64 bit_moves_rei[CASAS_DO_TABULEIRO];

int dtb_moves[CASAS_DO_TABULEIRO][9];
int cavalo_moves[CASAS_DO_TABULEIRO][9];
int rei_moves[CASAS_DO_TABULEIRO][9];

void init_cavalo_lookups(){
    int k, x = 0;
    
    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        k = 0;

        if (linhas[x] < 6 && colunas[x] < 7){
            cavalo_moves[x][k++] = x + 17;
        }
        if (linhas[x] < 7 && colunas[x] < 6){
            cavalo_moves[x][k++] = x + 10;
        }
        if (linhas[x] < 6 && colunas[x] > 0){
            cavalo_moves[x][k++] = x + 15;
        }
        if (linhas[x] < 7 && colunas[x] > 1){
            cavalo_moves[x][k++] = x + 6;
        }
        if (linhas[x] > 1 && colunas[x] < 7){
            cavalo_moves[x][k++] = x - 15;
        }
        if (linhas[x] > 0 && colunas[x] < 6){
            cavalo_moves[x][k++] = x - 6;
        }
        if (linhas[x] > 1 && colunas[x] > 0){
            cavalo_moves[x][k++] = x - 17;
        }
        if (linhas[x] > 0 && colunas[x] > 1){
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

        if (colunas[x] > 0){
            rei_moves[x][y++] = x-1;    
        }
        if (colunas[x] < 7){
            rei_moves[x][y++] = x+1;    
        }
        if (linhas[x] > 0){
            rei_moves[x][y++] = x-8;    
        }
        if (linhas[x] < 7){
            rei_moves[x][y++] = x+8;    
        }
        if (colunas[x] < 7 && linhas[x] < 7){
            rei_moves[x][y++] = x+9;    
        }
        if (colunas[x] > 0 && linhas[x] < 7){
            rei_moves[x][y++] = x+7;    
        }
        if (colunas[x] > 0 && linhas[x] > 0){
            rei_moves[x][y++] = x-9;    
        }
        if (colunas[x] < 7 && linhas[x] > 0){
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

        if (colunas[x] > 0){
            dtb_moves[x][OESTE] = x-1;
            rei_moves[x][y++] = x-1;    
        }
        if (colunas[x] < 7){
            dtb_moves[x][LESTE] = x+1;
            rei_moves[x][y++] = x+1;    
        }
        if (linhas[x] > 0){
            dtb_moves[x][SUL] = x-8;
            rei_moves[x][y++] = x-8;    
        }
        if (linhas[x] < 7){
            dtb_moves[x][NORTE] = x+8;
            rei_moves[x][y++] = x+8;    
        }
        if (colunas[x] < 7 && linhas[x] < 7){
            dtb_moves[x][NE] = x+9;
            rei_moves[x][y++] = x+9;    
        }
        if (colunas[x] > 0 && linhas[x] < 7){
            dtb_moves[x][NO] = x+7;
            rei_moves[x][y++] = x+7;    
        }
        if (colunas[x] > 0 && linhas[x] > 0){
            dtb_moves[x][SO] = x-9;
            rei_moves[x][y++] = x-9;    
        }
        if (colunas[x] < 7 && linhas[x] > 0){
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
    memset(bit_moves_cavalo, 0, sizeof(bit_moves_cavalo));
    memset(bit_moves_bispo, 0, sizeof(bit_moves_bispo));
    memset(bit_moves_torre, 0, sizeof(bit_moves_torre));
    memset(bit_moves_dama, 0, sizeof(bit_moves_dama));
    memset(bit_moves_rei, 0, sizeof(bit_moves_rei));

    init_cavalo_lookups();
    init_rei_lookups();
    init_dtb_lookups();
}