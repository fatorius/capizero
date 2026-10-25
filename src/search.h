#ifndef SEARCH
#define SEARCH

#include "consts.h"
#include "gen.h"

struct Estado;

namespace Search{
    extern int tempo_do_inicio, tempo_do_fim;

    // Preenche a tabela de redução LMR; chamar uma vez em Init::init().
    void init_lmr();

    void pensar(Estado &est, bool verbose);
    // null_permitido = false on the immediate child of a null-move so we don't
    // recurse two null-moves in a row (which would just compound the depth
    // skip with no information). Defaults to true so existing call sites
    // (real-move recursion, IID, qsearch) keep their current behavior.
    int pesquisa(Estado &est, int alpha, int beta, int profundidade, bool pv, bool null_permitido = true);
};

#endif