#ifndef SEARCH
#define SEARCH

#include "consts.h"
#include "gen.h"

extern int tempo_do_inicio, tempo_do_fim;
extern int lances_avaliados;

extern int historico_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
extern Gen::lance contraLance_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
extern Gen::lance killers_primarios[MAX_PLY];
extern Gen::lance killers_secundarios[MAX_PLY];

void pensar(bool verbose);
int pesquisa(int alpha, int beta, int profundidade, bool pv);

#endif