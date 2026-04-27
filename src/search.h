#ifndef SEARCH
#define SEARCH

#include "consts.h"
#include "gen.h"

namespace Search{
    extern int tempo_do_inicio, tempo_do_fim;
    extern int lances_avaliados;

    extern int historico_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
    extern Gen::lance contraLance_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
    extern Gen::lance killers_primarios[MAX_PLY];
    extern Gen::lance killers_secundarios[MAX_PLY];

    void pensar(bool verbose);
    // null_permitido = false on the immediate child of a null-move so we don't
    // recurse two null-moves in a row (which would just compound the depth
    // skip with no information). Defaults to true so existing call sites
    // (real-move recursion, IID, qsearch) keep their current behavior.
    int pesquisa(int alpha, int beta, int profundidade, bool pv, bool null_permitido = true);
};

#endif