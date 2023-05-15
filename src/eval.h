#ifndef EVAL
#define EVAL

#include "consts.h"

extern int score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
extern int reis_score_finais[LADOS][CASAS_DO_TABULEIRO];
extern int passados[LADOS][CASAS_DO_TABULEIRO];

void init_eval_tables();

#endif