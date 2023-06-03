#ifndef SEARCH
#define SEARCH

#include "consts.h"
#include "gen.h"

extern int tempo_do_inicio, tempo_do_fim;
extern int lances_avaliados;

extern int historico_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
extern lance contraLance_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];

void pensar(bool verbose);


#endif