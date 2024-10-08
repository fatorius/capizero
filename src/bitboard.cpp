#include "bitboard.h"

#include "update.h"

#include <string.h>
#include <stdint.h>
#include <cstdlib>

#ifdef MVSC
#include <nmmintrin.h>
#endif

Bitboard::u64 Bitboard::mask[CASAS_DO_TABULEIRO];
Bitboard::u64 Bitboard::not_mask[CASAS_DO_TABULEIRO];

Bitboard::u64 Bitboard::mask_cols[CASAS_DO_TABULEIRO];
Bitboard::u64 Bitboard::mask_rows[CASAS_DO_TABULEIRO];
Bitboard::u64 Bitboard::mask_isolados[CASAS_DO_TABULEIRO];

Bitboard::u64 Bitboard::mask_ala_do_rei;
Bitboard::u64 Bitboard::mask_ala_da_dama;

Bitboard::u64 Bitboard::bordas;
Bitboard::u64 Bitboard::bordas_neg;

Bitboard::u64 Bitboard::mask_passados[LADOS][CASAS_DO_TABULEIRO];
Bitboard::u64 Bitboard::mask_path[LADOS][CASAS_DO_TABULEIRO];

Bitboard::u64 Bitboard::not_coluna_a;
Bitboard::u64 Bitboard::not_coluna_h;

Bitboard::u64 Bitboard::bit_pieces[LADOS][TIPOS_DE_PIECES];
Bitboard::u64 Bitboard::bit_lados[LADOS];
Bitboard::u64 Bitboard::bit_total;

Bitboard::u64 Bitboard::bit_entre[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
Bitboard::u64 Bitboard::vetor_bits[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
Bitboard::u64 Bitboard::mask_vetores[CASAS_DO_TABULEIRO][8];

int Bitboard::fileiras[LADOS][CASAS_DO_TABULEIRO];

int Bitboard::tabuleiro[CASAS_DO_TABULEIRO];

void Bitboard::init_bits(){
    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        Bitboard::set_bit(mask[casa], casa);
        not_mask[casa] = ~mask[casa];

        for (int casa2 = 0; casa2 < CASAS_DO_TABULEIRO; casa2++){
            if (Consts::colunas[casa] == Consts::colunas[casa2]){
                Bitboard::set_bit(mask_cols[casa], casa2);

                if (Consts::linhas[casa] < Consts::linhas[casa2]){
                    Bitboard::set_bit(mask_path[BRANCAS][casa], casa2);
                    Bitboard::set_bit(mask_passados[BRANCAS][casa], casa2);
                }
                if (Consts::linhas[casa] > Consts::linhas[casa2]){
                    Bitboard::set_bit(mask_path[PRETAS][casa], casa2);
                    Bitboard::set_bit(mask_passados[PRETAS][casa], casa2);
                }
            }

            if (Consts::linhas[casa] == Consts::linhas[casa2]){
                Bitboard::set_bit(mask_rows[casa], casa2);
            }

            if (abs(Consts::colunas[casa] - Consts::colunas[casa2]) == 1){
                if (Consts::linhas[casa] < Consts::linhas[casa2]){
                    Bitboard::set_bit(mask_passados[BRANCAS][casa], casa2);
                }
                else if (Consts::linhas[casa] > Consts::linhas[casa2]){
                    Bitboard::set_bit(mask_passados[PRETAS][casa], casa2);
                }
            }

            if (abs(Consts::colunas[casa] - Consts::colunas[casa2]) == 1){
                Bitboard::set_bit(mask_isolados[casa], casa2);
            }
        }

        fileiras[BRANCAS][casa] = Consts::linhas[casa];
        fileiras[PRETAS][casa] = 7 - Consts::linhas[casa];

        if (Consts::colunas[casa] < COLUNA_D){
            Bitboard::set_bit(mask_ala_da_dama, casa);
        }

        if (Consts::colunas[casa] > COLUNA_E){
            Bitboard::set_bit(mask_ala_do_rei, casa);
        }


    }

    not_coluna_a = ~mask_cols[COLUNA_A];
    not_coluna_h = ~mask_cols[COLUNA_H];

    bordas = mask_cols[COLUNA_A] | mask_cols[COLUNA_H] | mask_rows[LINHA_1] | mask_rows[LINHA_8];
    bordas_neg = ~bordas;
}

void Bitboard::init_vetores(){
    int casaInicio, casaDestino;
    int bit;

    for (casaInicio = 0; casaInicio < CASAS_DO_TABULEIRO; casaInicio++){
        for (casaDestino = 0; casaDestino < CASAS_DO_TABULEIRO; casaDestino++){
            if (Consts::linhas[casaInicio] == Consts::linhas[casaDestino]){
                if (casaDestino > casaInicio){
                    for (bit = casaInicio + 1; bit < casaDestino; bit++){
                        Bitboard::set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaInicio + 1; bit <= casaDestino; bit++){
                        Bitboard::set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
                else{
                    for (bit = casaDestino + 1; bit < casaInicio; bit++){
                        Bitboard::set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaDestino; bit < casaInicio; bit++){
                        Bitboard::set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
            }

            if (Consts::colunas[casaInicio] == Consts::colunas[casaDestino]){
                if (casaDestino > casaInicio){
                    for (bit = casaInicio + 8; bit < casaDestino; bit += 8){
                        Bitboard::set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaInicio + 8; bit <= casaDestino; bit += 8){
                        Bitboard::set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
                else{
                    for (bit = casaDestino + 8; bit < casaInicio; bit += 8){
                        Bitboard::set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaDestino; bit < casaInicio; bit += 8){
                        Bitboard::set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
            }

            if (Consts::no_diag[casaInicio] == Consts::no_diag[casaDestino]){
                if (casaDestino > casaInicio){
                    for (bit = casaInicio + 7; bit < casaDestino; bit += 7){
                        Bitboard::set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaInicio + 7; bit <= casaDestino; bit += 7){
                        Bitboard::set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
                else{
                    for (bit = casaDestino + 7; bit < casaInicio; bit += 7){
                        Bitboard::set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaDestino; bit < casaInicio; bit += 7){
                        Bitboard::set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
            }
            
            if (Consts::ne_diag[casaInicio] == Consts::ne_diag[casaDestino]){
                if (casaDestino > casaInicio){
                    for (bit = casaInicio + 9; bit < casaDestino; bit += 9){
                        Bitboard::set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaInicio + 9; bit <= casaDestino; bit += 9){
                        Bitboard::set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
                else{
                    for (bit = casaDestino + 9; bit < casaInicio; bit += 9){
                        Bitboard::set_bit(bit_entre[casaInicio][casaDestino], bit);
                    }
                    for (bit = casaDestino; bit < casaInicio; bit += 9){
                        Bitboard::set_bit(vetor_bits[casaInicio][casaDestino], bit);
                    }
                }
            }
        }
    }

    for (casaInicio = 0; casaInicio < CASAS_DO_TABULEIRO; casaInicio++){
        mask_vetores[casaInicio][NORTE] = vetor_bits[casaInicio][56+Consts::colunas[casaInicio]];
        mask_vetores[casaInicio][SUL] = vetor_bits[casaInicio][Consts::colunas[casaInicio]];
        mask_vetores[casaInicio][OESTE] = vetor_bits[casaInicio][Consts::linhas[casaInicio] * 8];
        mask_vetores[casaInicio][LESTE] = vetor_bits[casaInicio][(Consts::linhas[casaInicio] * 8) + 7];

        if (Consts::colunas[casaInicio] > COLUNA_A && Consts::linhas[casaInicio] < COLUNA_H){
            mask_vetores[casaInicio][NO] = vetor_bits[casaInicio][Bitboard::obter_borda(casaInicio, 7)];
        }
        if (Consts::colunas[casaInicio] < COLUNA_H && Consts::linhas[casaInicio] < COLUNA_H){
            mask_vetores[casaInicio][NE] = vetor_bits[casaInicio][Bitboard::obter_borda(casaInicio, 9)];
        }
        if (Consts::linhas[casaInicio] > COLUNA_A && Consts::colunas[casaInicio] > COLUNA_A){
            mask_vetores[casaInicio][SO] = vetor_bits[casaInicio][Bitboard::obter_borda(casaInicio, -9)];
        }
        if (Consts::linhas[casaInicio] > COLUNA_A && Consts::colunas[casaInicio] < COLUNA_H){
            mask_vetores[casaInicio][SE] = vetor_bits[casaInicio][Bitboard::obter_borda(casaInicio, -7)];
        }
    }
}

void Bitboard::init_board(){
    memset(bit_pieces, 0, sizeof(bit_pieces));
    memset(bit_lados, 0, sizeof(bit_lados));
    bit_total = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; ++casa) {
        tabuleiro[casa] = VAZIO;

        if(Consts::tabuleiro_inicial[casa] < 6){
            Update::adicionar_piece(Consts::cor_inicial[casa], Consts::tabuleiro_inicial[casa], casa);
        }
    }
}

void Bitboard::set_bit(u64 &bitboard, int bit){
    bitboard |= (UINT64_C(1) << bit);
}

int Bitboard::obter_borda(int casa, int soma){
    do {
        casa += soma;
    }
    while (Consts::colunas[casa] > COLUNA_A && Consts::colunas[casa] < COLUNA_H && Consts::linhas[casa] > COLUNA_A && Consts::linhas[casa] < COLUNA_H);

    return casa;
}

int Bitboard::bitscan(u64 b){
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
        return Consts::lsb_64_table[folded * 0x78291ACF >> 26];
    #endif
}

int Bitboard::popcount(u64 b){
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