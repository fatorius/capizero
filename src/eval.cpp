#include "eval.h"

#include "bitboard.h"
#include "consts.h"
#include "game.h"

int score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
int reis_score_finais[LADOS][CASAS_DO_TABULEIRO];
int passados[LADOS][CASAS_DO_TABULEIRO];

int peao_mat[LADOS];
int piece_mat[LADOS];

int peao_ala_da_dama[LADOS],peao_ala_do_rei[LADOS];

void init_eval_tables(){
    for (int x = 0; x < CASAS_DO_TABULEIRO; x++){
        score_casas[BRANCAS][P][x] = peao_score[x] + VALOR_PEAO;
        score_casas[BRANCAS][C][x] = cavalo_score[x] + VALOR_CAVALO;
        score_casas[BRANCAS][B][x] = bispo_score[x] + VALOR_BISPO;
        score_casas[BRANCAS][T][x] = torre_score[x] + VALOR_TORRE;
        score_casas[BRANCAS][D][x] = dama_score[x] + VALOR_DAMA;
        score_casas[BRANCAS][R][x] = rei_score[x];

        score_casas[PRETAS][P][x] = peao_score[flip[x]] + VALOR_PEAO;
        score_casas[PRETAS][C][x] = cavalo_score[flip[x]] + VALOR_CAVALO;
        score_casas[PRETAS][B][x] = bispo_score[flip[x]] + VALOR_BISPO;
        score_casas[PRETAS][T][x] = torre_score[flip[x]] + VALOR_TORRE;
        score_casas[PRETAS][D][x] = dama_score[flip[x]] + VALOR_DAMA;
        score_casas[PRETAS][R][x] = rei_score[flip[x]];
        
        reis_score_finais[BRANCAS][x] = rei_finais_score[x] - score_casas[BRANCAS][R][x];
        reis_score_finais[PRETAS][x] = rei_finais_score[x] - score_casas[PRETAS][R][x];

        passados[BRANCAS][x] = peao_passado_score[flip[x]];
        passados[PRETAS][x] = peao_passado_score[x];
    }
}

void atualizar_materiais(){
    int cor;

    peao_mat[BRANCAS] = 0;
    peao_mat[PRETAS] = 0;

    piece_mat[BRANCAS] = 0;
    piece_mat[PRETAS] = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        if (tabuleiro[casa] < VAZIO){
            if (bit_lados[BRANCAS] & mask[casa]){
                cor = BRANCAS;
            }
            else{
                cor = PRETAS;
            }

            if (tabuleiro[casa] == P){
                peao_mat[cor] += VALOR_PEAO;
            }
            else{
                piece_mat[cor] += pieces_valor[tabuleiro[casa]];
            }
        }
    }
}

int avaliar_peao(const int l, const int casa){
    int score = 0;
    int xl = l^1;

    if (!(mask_passados[l][casa] & bit_pieces[xl][P]) && !(mask_path[l][casa] & bit_pieces[l][P])){
        score += passados[l][casa];
    }

    if ((mask_isolados[casa] & bit_pieces[l][P]) == 0){
        score -= ISOLADO_SCORE;
    }

    peao_ala_do_rei[l] = defesa_ala_do_rei[l][casa];
    peao_ala_da_dama[l] = defesa_ala_da_dama[l][casa];

    return score;
}

int avaliar_torre(const int l, const int casa){
    int score = 0;

    if (!(mask_cols[casa] & bit_pieces[l][P])){
        score = COLUNA_SEMI_ABERTA_BONUS;

        if (!(mask_cols[casa] & bit_pieces[l^1][P])){
            return COLUNA_ABERTA_BONUS;
        }
    }

    return score;
}

int avaliar(){
    int score[LADOS] = {0, 0};

    peao_ala_da_dama[BRANCAS] = 0;
    peao_ala_do_rei[BRANCAS] = 0;
    peao_ala_da_dama[PRETAS] = 0;
    peao_ala_do_rei[PRETAS] = 0;

    u64 t1;
    int casa;

    for (int l = 0; l < LADOS; l++){
        
        t1 = bit_pieces[l][P];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas[l][P][casa];
            score[l] += avaliar_peao(l, casa);
        }

        t1 = bit_pieces[l][C];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas[l][C][casa];
        }

        t1 = bit_pieces[l][B];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas[l][B][casa];
        }

        t1 = bit_pieces[l][T];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas[l][T][casa];
            score[l] += avaliar_torre(l, casa);
        }

        t1 = bit_pieces[l][D];
        while (t1){
            casa = bitscan(t1);
            t1 &= not_mask[casa];

            score[l] += score_casas[l][D][casa];
        }
    }

    if (bit_pieces[PRETAS][D] == 0){
        score[BRANCAS] += reis_score_finais[BRANCAS][bitscan(bit_pieces[BRANCAS][R])];
    }
    else{
        if (bit_pieces[BRANCAS][R] & mask_ala_do_rei){
            score[BRANCAS] += peao_ala_do_rei[BRANCAS];
        }
        else if (bit_pieces[BRANCAS][R] & mask_ala_da_dama){
            score[BRANCAS] += peao_ala_da_dama[BRANCAS];
        }
    }

    if (bit_pieces[BRANCAS][D] == 0){
        score[PRETAS] += reis_score_finais[PRETAS][bitscan(bit_pieces[PRETAS][R])];
    }
    else {
        if (bit_pieces[PRETAS][R] & mask_ala_do_rei){
            score[PRETAS] += peao_ala_do_rei[PRETAS];
        }
        else if (bit_pieces[PRETAS][R] & mask_ala_da_dama){
            score[PRETAS] += peao_ala_da_dama[PRETAS];
        }
    }

    return score[lado] - score[xlado];
}