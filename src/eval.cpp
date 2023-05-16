#include "eval.h"

int score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
int reis_score_finais[LADOS][CASAS_DO_TABULEIRO];
int passados[LADOS][CASAS_DO_TABULEIRO];


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