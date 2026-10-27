#ifndef SEARCH
#define SEARCH

#include <atomic>

#include "consts.h"
#include "gen.h"

struct Estado;

namespace Search{
    extern int tempo_do_inicio, tempo_do_fim;

    // Pedido de parada da busca, compartilhado entre threads.
    extern std::atomic<bool> parar_pesquisa;

    // Preenche a tabela de redução LMR; chamar uma vez em Init::init().
    void init_lmr();

    // zerar_parada = false quando quem chama já zerou parar_pesquisa antes de
    // liberar a thread de busca (Threads::iniciar_busca), para que um stop que
    // chegue antes de a busca começar não se perca.
    void pensar(Estado &est, bool verbose, bool zerar_parada = true);
    // null_permitido = false on the immediate child of a null-move so we don't
    // recurse two null-moves in a row (which would just compound the depth
    // skip with no information). Defaults to true so existing call sites
    // (real-move recursion, IID, qsearch) keep their current behavior.
    int pesquisa(Estado &est, int alpha, int beta, int profundidade, bool pv, bool null_permitido = true);
};

#endif