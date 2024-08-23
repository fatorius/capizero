#ifndef EVAL
#define EVAL

#include "consts.h"

namespace Eval{
    extern int score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
    extern int reis_score_finais[LADOS][CASAS_DO_TABULEIRO];
    extern int passados[LADOS][CASAS_DO_TABULEIRO];

    extern int peao_mat[2];
    extern int piece_mat[2];

    void init_eval_tables();
    void atualizar_materiais();

    int avaliar();
};

#endif