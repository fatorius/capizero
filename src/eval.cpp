#include "eval.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#if defined(__BMI2__) && !defined(NOT_USE_PEXT)
    #include <immintrin.h>
    #define USE_PEXT
#else
    #define USE_MAGIC_HASHING
#endif

#include "magics.h"
#include "bitboard.h"
#include "consts.h"
#include "game.h"
#include "gen.h"

int score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
int score_casas_finais[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
int reis_score_finais[LADOS][CASAS_DO_TABULEIRO];
int passados[LADOS][CASAS_DO_TABULEIRO];

int peao_mat[LADOS];
int piece_mat[LADOS];

int peao_ala_da_dama[LADOS],peao_ala_do_rei[LADOS];

void init_eval_tables(){
    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        score_casas[BRANCAS][P][casa] = peao_score[casa] + VALOR_PEAO;
        score_casas[BRANCAS][C][casa] = cavalo_score[casa] + VALOR_CAVALO;
        score_casas[BRANCAS][B][casa] = bispo_score[casa] + VALOR_BISPO;
        score_casas[BRANCAS][T][casa] = torre_score[casa] + VALOR_TORRE;
        score_casas[BRANCAS][D][casa] = dama_score[casa] + VALOR_DAMA;
        score_casas[BRANCAS][R][casa] = rei_score[casa];

        score_casas_finais[BRANCAS][P][casa] = peao_score_finais[casa] + VALOR_PEAO_FINAIS;
        score_casas_finais[BRANCAS][C][casa] = cavalo_score_finais[casa] + VALOR_CAVALO_FINAIS;
        score_casas_finais[BRANCAS][B][casa] = bispo_score_finais[casa] + VALOR_BISPO_FINAIS;
        score_casas_finais[BRANCAS][T][casa] = torre_score_finais[casa] + VALOR_TORRE_FINAIS;
        score_casas_finais[BRANCAS][D][casa] = dama_score_finais[casa] + VALOR_DAMA_FINAIS;

        score_casas[PRETAS][P][casa] = peao_score[flip[casa]] + VALOR_PEAO;
        score_casas[PRETAS][C][casa] = cavalo_score[flip[casa]] + VALOR_CAVALO;
        score_casas[PRETAS][B][casa] = bispo_score[flip[casa]] + VALOR_BISPO;
        score_casas[PRETAS][T][casa] = torre_score[flip[casa]] + VALOR_TORRE;
        score_casas[PRETAS][D][casa] = dama_score[flip[casa]] + VALOR_DAMA;
        score_casas[PRETAS][R][casa] = rei_score[flip[casa]];
        
        score_casas_finais[PRETAS][P][casa] = peao_score_finais[flip[casa]] + VALOR_PEAO_FINAIS;
        score_casas_finais[PRETAS][C][casa] = cavalo_score_finais[flip[casa]] + VALOR_CAVALO_FINAIS;
        score_casas_finais[PRETAS][B][casa] = bispo_score_finais[flip[casa]] + VALOR_BISPO_FINAIS;
        score_casas_finais[PRETAS][T][casa] = torre_score_finais[flip[casa]] + VALOR_TORRE_FINAIS;
        score_casas_finais[PRETAS][D][casa] = dama_score_finais[flip[casa]] + VALOR_DAMA_FINAIS;        

        reis_score_finais[BRANCAS][casa] = rei_finais_score[casa] - score_casas[BRANCAS][R][casa];
        reis_score_finais[PRETAS][casa] = rei_finais_score[casa] - score_casas[PRETAS][R][casa];

        passados[BRANCAS][casa] = peao_passado_score[flip[casa]];
        passados[PRETAS][casa] = peao_passado_score[casa];
    }
}

void atualizar_materiais(){
    int cor;

    peao_mat[BRANCAS] = 0;
    peao_mat[PRETAS] = 0;

    piece_mat[BRANCAS] = 0;
    piece_mat[PRETAS] = 0;

    u64 tt = bit_total;
    int casa;

    while(tt){
        casa = bitscan(tt);
        tt &= not_mask[casa];

        if (bit_lados[BRANCAS] & mask[casa]){
            cor = BRANCAS;
        }
        else{
            cor = PRETAS;
        }

        piece_mat[cor] += valor_material[tabuleiro[casa]];
    }
}

int avaliar_peao(const int l, const int casa){
    int score = 0;
    int xl = l^1;

    if (!(mask_passados[l][casa] & bit_pieces[xl][P])){
        score += passados[l][casa];
    }

    if ((mask_isolados[casa] & bit_pieces[l][P]) == 0){
        score -= ISOLADO_SCORE;
    }
    else{
        if (mask[peao_esquerda[xl][casa]] & bit_pieces[l][P]){
            score += PEAO_PROTEGIDO_SCORE;
        }
        if (mask[peao_direita[xl][casa]] & bit_pieces[l][P]){
            score += PEAO_PROTEGIDO_SCORE;
        }
    }

    peao_ala_do_rei[l] += defesa_ala_do_rei[l][casa];
    peao_ala_da_dama[l] += defesa_ala_da_dama[l][casa];

    return score;
}

int avaliar_peao_finais(const int l, const int casa){
    int score = 0;
    int xl = l^1;

    if (!(mask_passados[l][casa] & bit_pieces[xl][P])){
        score += passados[l][casa];
    }

    if (mask[peao_esquerda[xl][casa]] & bit_pieces[l][P]){
        score += PEAO_PROTEGIDO_SCORE_FINAIS;
    }
    if (mask[peao_direita[xl][casa]] & bit_pieces[l][P]){
        score += PEAO_PROTEGIDO_SCORE_FINAIS;
    }

    return score;
}

int atividade_cavalo(const int l, const int casa){
    u64 pulos_cavalo = bit_moves_cavalo[casa] & bit_lados[l^1];

    return (popcount(pulos_cavalo) * ATIVIDADE_CAVALO);
}

int atividade_bispo(const int l, const int casa){
    #ifdef USE_PEXT
        u64 ataques_bispo = bit_magicas_bispo[casa][_pext_u64(bit_total, bit_casas_relevantes_bispo[casa])];
    #else
        u64 ataques_bispo = bit_magicas_bispo[casa][((bit_total & bit_casas_relevantes_bispo[casa]) * magicas_bispos[casa]) >> (bits_indices_bispos[casa])];
    #endif

    return (popcount(ataques_bispo) * ATIVIDADE_BISPO);
}

int atividade_torre(const int l, const int casa){
    #ifdef USE_PEXT
        u64 ataques_torre = bit_magicas_torre[casa][_pext_u64(bit_total, bit_casas_relevantes_torres[casa])];
    #else
        u64 ataques_torre = bit_magicas_torre[casa][((bit_total & bit_casas_relevantes_torres[casa]) * magicas_torres[casa]) >> (bits_indices_torres[casa])];
    #endif

    return (popcount(ataques_torre) * ATIVIDADE_TORRE);
}

int atividade_dama(const int l, const int casa){
    #ifdef USE_PEXT
        u64 ataques_dama = bit_magicas_bispo[casa][_pext_u64(bit_total, bit_casas_relevantes_bispo[casa])] | bit_magicas_torre[casa][_pext_u64(bit_total, bit_casas_relevantes_torres[casa])];
    #else
        u64 ataques_dama = bit_magicas_bispo[casa][((bit_total & bit_casas_relevantes_bispo[casa]) * magicas_bispos[casa]) >> (bits_indices_bispos[casa])] | bit_magicas_torre[casa][((bit_total & bit_casas_relevantes_torres[casa]) * magicas_torres[casa]) >> (bits_indices_torres[casa])];
    #endif

    return (popcount(ataques_dama) * ATIVIDADE_DAMA);
}

int avaliar_torre(const int l, const int casa){
    int xl = l^1;

    if (!(mask_cols[casa] & bit_pieces[l][P])){
        if (!(mask_cols[casa] & bit_pieces[xl][P])){
            return COLUNA_ABERTA_BONUS;
        }

        return COLUNA_SEMI_ABERTA_BONUS;
    }
    else if (!(mask_cols[casa] & bit_pieces[xl][P])){
        return COLUNA_SEMI_ABERTA_BONUS;
    }

    return 0;
}

int avaliar_dama(const int l, const int casa){
    int xl = l^1;

    if (!(mask_cols[casa] & bit_pieces[l][P])){
        if (!(mask_cols[casa] & bit_pieces[xl][P])){
            return COLUNA_ABERTA_BONUS_DAMA;
        }

        return COLUNA_SEMI_ABERTA_BONUS_DAMA;
    }
    else if (!(mask_cols[casa] & bit_pieces[xl][P])){
        return COLUNA_SEMI_ABERTA_BONUS_DAMA;
    }

    return 0;
}

int seguranca_do_rei(const int l){
    if (bit_pieces[l][R] & mask_ala_do_rei){
        return peao_ala_do_rei[l];
    }
    else if (bit_pieces[l][R] & mask_ala_da_dama){
        return peao_ala_da_dama[l];
    }

    return REI_NO_CENTRO_MEIO_JOGO;
}

int avaliar_meio_jogo(bool lazy){
    int score[LADOS] = {0, 0};

    memset(peao_ala_do_rei, 0, sizeof(peao_ala_do_rei));
    memset(peao_ala_da_dama, 0, sizeof(peao_ala_da_dama));
    
    u64 t1;
    int casa;

    for (int l = 0; l < LADOS; l++){
        t1 = bit_pieces[l][P];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas[l][P][casa];

            if (!lazy){
                score[l] += avaliar_peao(l, casa);
            }
        }

        t1 = bit_pieces[l][C];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas[l][C][casa];

            if (!lazy){
                score[l] += atividade_cavalo(l, casa);
            }
        }

        t1 = bit_pieces[l][B];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas[l][B][casa];

            if (!lazy){
                score[l] += atividade_bispo(l, casa);
            }
        }

        t1 = bit_pieces[l][T];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas[l][T][casa];

            if (!lazy){
                score[l] += avaliar_torre(l, casa);
                score[l] += atividade_torre(l, casa); 
            }
        }

        t1 = bit_pieces[l][D];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas[l][D][casa];

            if (!lazy){
                score[l] += avaliar_dama(l, casa);
                score[l] += atividade_dama(l, casa);
            }
        }

        if (!lazy){
            score[l] += seguranca_do_rei(l);
            score[l] += rei_score[bitscan(bit_pieces[l][R])];
        }
    }

    return score[lado] - score[xlado];
}

int avaliar_finais(bool lazy){
    int score[LADOS] = {0, 0};

    u64 t1;
    int casa;

    for (int l = 0; l < LADOS; l++){

        t1 = bit_pieces[l][P];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas_finais[l][P][casa];
            score[l] += avaliar_peao_finais(l, casa);
        }

        t1 = bit_pieces[l][C];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas_finais[l][C][casa];
        }

        t1 = bit_pieces[l][B];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas_finais[l][B][casa];
        }

        t1 = bit_pieces[l][T];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas_finais[l][T][casa];
        }

        t1 = bit_pieces[l][D];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas_finais[l][D][casa];
        }

        score[l] += rei_finais_score[bitscan(bit_pieces[l][R])];
    }

    return score[lado] - score[xlado];
}

int diff_material(){
    return (
        (popcount(bit_pieces[BRANCAS][P]) - popcount(bit_pieces[PRETAS][P]))
        +
        ((popcount(bit_pieces[BRANCAS][C]) - popcount(bit_pieces[PRETAS][C])) * 3)
        +
        ((popcount(bit_pieces[BRANCAS][B]) - popcount(bit_pieces[PRETAS][B])) * 3)
        +
        ((popcount(bit_pieces[BRANCAS][T]) - popcount(bit_pieces[PRETAS][T])) * 5)
        +
        ((popcount(bit_pieces[BRANCAS][D]) - popcount(bit_pieces[PRETAS][D])) * 9)
    );
}

int material_total(){
    return popcount(bit_total);
}

int avaliar(){
    int eval_material = diff_material(); 
    int material = material_total();

    bool lazy_eval = false;

    if (abs(eval_material) > VANTAGEM_DECISIVA){
        lazy_eval = true;
    }
/*
    printf("====================\n");
    printf("Material: %d \n", material);
    printf("Material diff: %d\n", eval_material);
    printf("Peso meio jogo: %.2f \n", peso_eval_meio_jogo[material]);
    printf("Peso final: %.2f \n", peso_eval_final[material]);
    printf("Meio jogo: %d \n", avaliar_meio_jogo(lazy_eval));
    printf("Final: %d \n", avaliar_finais(lazy_eval));
    printf("Eval meio jogo: %f \n", avaliar_meio_jogo(lazy_eval) * peso_eval_meio_jogo[material]);
    printf("Eval final: %f \n", avaliar_finais(lazy_eval) * peso_eval_final[material]);
    printf("Avaliação: %f \n",((avaliar_meio_jogo(lazy_eval) * peso_eval_meio_jogo[material])
    +
    (avaliar_finais(lazy_eval) * peso_eval_final[material])));
    printf("====================\n");
*/
    return 
    ((avaliar_meio_jogo(lazy_eval) * peso_eval_meio_jogo[material])
    +
    (avaliar_finais(lazy_eval) * peso_eval_final[material]));
}
