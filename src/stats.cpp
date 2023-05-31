#include "stats.h"

#include <stdio.h>

#include "gen.h"
#include "interface.h"
#include "update.h"
#include "game.h"
#include "eval.h"
#include "search.h"

void Stats::movegen_performance_test(){
    int tempoInicial = obter_tempo();

    unsigned long long lances_gerados = perft_node(5);

    int delta = obter_tempo() - tempoInicial;

    int nps_perft;

    nps_perft = (double) lances_gerados / (double) delta;

    printf("Gerador de lances: %dK lances por segundo\n", nps_perft);
}

void Stats::search_performance_test(){
    tempo_maximo = 5000;
    profundidade_maxima = MAX_PLY;
    tempo_fixo = true;
    profundidade_fixa = false;

    int tempoInicial = obter_tempo();

    lance_computador(false);

    int delta = obter_tempo() - tempoInicial;

    int nps_search;

    nps_search = (double) lances_avaliados / (double) delta;

    printf("Pesquisa de lances: %dK posições por segundo\n", nps_search);

    novo_jogo();
}

void Stats::eval_performance_test(){
    int tempoInicial = obter_tempo();

    long avals = 0;

    while ((obter_tempo() - tempoInicial) < 5000){

        for (int i = 0; i < 4000; i++){
            avaliar();
        }
        
        avals += 4000;
    }

    int nps_eval = (double) avals / (double) 5000;
    
    printf("Avaliações de posições: %dK posições por segundo\n\n", nps_eval);
}