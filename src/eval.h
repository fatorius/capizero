#ifndef EVAL
#define EVAL

#include "consts.h"

extern int score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
extern int reis_score_finais[LADOS][CASAS_DO_TABULEIRO];
extern int passados[LADOS][CASAS_DO_TABULEIRO];

extern int peao_mat[2];
extern int piece_mat[2];

extern int valores_atividade_bispo_meio_jogo[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];
extern int valores_atividade_bispo_finais[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];

extern int valores_atividade_torre_meio_jogo[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];
extern int valores_atividade_torre_finais[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];

extern int valores_atividade_dama_meio_jogo[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];
extern int valores_atividade_dama_finais[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];

extern int contagem_bispo[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];
extern int contagem_torre[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];

void init_eval_tables();
void atualizar_materiais();

int avaliar();

#endif