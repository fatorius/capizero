#include "bitboard.h"

#include "update.h"

#include <string.h>
#include <stdint.h>
#include <cstdlib>

#ifdef MVSC
#include <nmmintrin.h>
#endif

u64 mask[CASAS_DO_TABULEIRO];
u64 not_mask[CASAS_DO_TABULEIRO];

u64 mask_cols[CASAS_DO_TABULEIRO];
u64 mask_rows[CASAS_DO_TABULEIRO];
u64 mask_isolados[CASAS_DO_TABULEIRO];

u64 mask_ala_do_rei;
u64 mask_ala_da_dama;

u64 bordas;
u64 bordas_neg;

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

void init_bits(){
    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        set_bit(mask[casa], casa);
        not_mask[casa] = ~mask[casa];

        for (int casa2 = 0; casa2 < CASAS_DO_TABULEIRO; casa2++){
            if (colunas[casa] == colunas[casa2]){
                set_bit(mask_cols[casa], casa2);

                if (linhas[casa] < linhas[casa2]){
                    set_bit(mask_path[BRANCAS][casa], casa2);
                    set_bit(mask_passados[BRANCAS][casa], casa2);
                }
                if (linhas[casa] > linhas[casa2]){
                    set_bit(mask_path[PRETAS][casa], casa2);
                    set_bit(mask_passados[PRETAS][casa], casa2);
                }
            }

            if (linhas[casa] == linhas[casa2]){
                set_bit(mask_rows[casa], casa2);
            }

            if (abs(colunas[casa] - colunas[casa2]) == 1){
                if (linhas[casa] < linhas[casa2]){
                    set_bit(mask_passados[BRANCAS][casa], casa2);
                }
                else if (linhas[casa] > linhas[casa2]){
                    set_bit(mask_passados[PRETAS][casa], casa2);
                }
            }

            if (abs(colunas[casa] - colunas[casa2]) == 1){
                set_bit(mask_isolados[casa], casa2);
            }
        }

        fileiras[BRANCAS][casa] = linhas[casa];
        fileiras[PRETAS][casa] = 7 - linhas[casa];

        if (colunas[casa] < COLUNA_D){
            set_bit(mask_ala_da_dama, casa);
        }

        if (colunas[casa] > COLUNA_E){
            set_bit(mask_ala_do_rei, casa);
        }


    }

    not_coluna_a = ~mask_cols[COLUNA_A];
    not_coluna_h = ~mask_cols[COLUNA_H];

    bordas = mask_cols[COLUNA_A] | mask_cols[COLUNA_H] | mask_rows[LINHA_1] | mask_rows[LINHA_8];
    bordas_neg = ~bordas;
}

void init_vetores(){
    int casaInicio, casaDestino;
    int bit;

    for (casaInicio = 0; casaInicio < CASAS_DO_TABULEIRO; casaInicio++){
        for (casaDestino = 0; casaDestino < CASAS_DO_TABULEIRO; casaDestino++){
            if (linhas[casaInicio] == linhas[casaDestino]){
                if (casaDestino > casaInicio){
                    for (bit = casaInicio + 1; bit < casaDestino; bit++){
                        set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaInicio + 1; bit <= casaDestino; bit++){
                        set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
                else{
                    for (bit = casaDestino + 1; bit < casaInicio; bit++){
                        set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaDestino; bit < casaInicio; bit++){
                        set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
            }

            if (colunas[casaInicio] == colunas[casaDestino]){
                if (casaDestino > casaInicio){
                    for (bit = casaInicio + 8; bit < casaDestino; bit += 8){
                        set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaInicio + 8; bit <= casaDestino; bit += 8){
                        set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
                else{
                    for (bit = casaDestino + 8; bit < casaInicio; bit += 8){
                        set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaDestino; bit < casaInicio; bit += 8){
                        set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
            }

            if (no_diag[casaInicio] == no_diag[casaDestino]){
                if (casaDestino > casaInicio){
                    for (bit = casaInicio + 7; bit < casaDestino; bit += 7){
                        set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaInicio + 7; bit <= casaDestino; bit += 7){
                        set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
                else{
                    for (bit = casaDestino + 7; bit < casaInicio; bit += 7){
                        set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaDestino; bit < casaInicio; bit += 7){
                        set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
            }
            
            if (ne_diag[casaInicio] == ne_diag[casaDestino]){
                if (casaDestino > casaInicio){
                    for (bit = casaInicio + 9; bit < casaDestino; bit += 9){
                        set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaInicio + 9; bit <= casaDestino; bit += 9){
                        set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
                else{
                    for (bit = casaDestino + 9; bit < casaInicio; bit += 9){
                        set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaDestino; bit < casaInicio; bit += 9){
                        set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
            }
        }
    }

    for (casaInicio = 0; casaInicio < CASAS_DO_TABULEIRO; casaInicio++){
        mask_vetores[casaInicio][NORTE] = vetor_bits[casaInicio][56+colunas[casaInicio]];
        mask_vetores[casaInicio][SUL] = vetor_bits[casaInicio][colunas[casaInicio]];
        mask_vetores[casaInicio][OESTE] = vetor_bits[casaInicio][linhas[casaInicio] * 8];
        mask_vetores[casaInicio][LESTE] = vetor_bits[casaInicio][(linhas[casaInicio] * 8) + 7];

        if (colunas[casaInicio] > COLUNA_A && linhas[casaInicio] < COLUNA_H){
            mask_vetores[casaInicio][NO] = vetor_bits[casaInicio][obter_borda(casaInicio, 7)];
        }
        if (colunas[casaInicio] < COLUNA_H && linhas[casaInicio] < COLUNA_H){
            mask_vetores[casaInicio][NE] = vetor_bits[casaInicio][obter_borda(casaInicio, 9)];
        }
        if (linhas[casaInicio] > COLUNA_A && colunas[casaInicio] > COLUNA_A){
            mask_vetores[casaInicio][SO] = vetor_bits[casaInicio][obter_borda(casaInicio, -9)];
        }
        if (linhas[casaInicio] > COLUNA_A && colunas[casaInicio] < COLUNA_H){
            mask_vetores[casaInicio][SE] = vetor_bits[casaInicio][obter_borda(casaInicio, -7)];
        }
    }
}

void init_board(){
    memset(bit_pieces, 0, sizeof(bit_pieces));
    memset(bit_lados, 0, sizeof(bit_lados));
    bit_total = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; ++casa) {
        tabuleiro[casa] = VAZIO;

        if(tabuleiro_inicial[casa] < 6){
            adicionar_piece(cor_inicial[casa], tabuleiro_inicial[casa], casa);
        }
    }
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
    #ifdef GNUC
        return __builtin_ctzll(b);
    #elif defined(MVSC) 
        unsigned long idx;
        _BitScanForward64(&idx, b);
        return idx;
    #else
        unsigned int folded;
        b ^= b - 1;
        folded = (int) b ^ (b >> 32);
        return lsb_64_table[folded * 0x78291ACF >> 26];
    #endif
}

int popcount(u64 b){
    #ifdef GNUC
        return __builtin_popcountll(b);
    #elif defined(MVSC)
        return _mm_popcnt_u64(b);
    #else
        int count = 0;
        int i;
        while (b) {
            i = bitscan(b);
            b &= not_mask[i];

            count++;
        }
        return count;
    #endif
}