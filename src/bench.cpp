#include "bench.h"

#include <stdio.h>

#include "gen.h"
#include "interface.h"
#include "update.h"
#include "game.h"
#include "eval.h"
#include "search.h"
#include "hash.h"

void Bench::movegen_performance_test(){
    int tempoInicial = Interface::obter_tempo();

    unsigned long long lances_gerados = Gen::perft_node(5);

    int delta = Interface::obter_tempo() - tempoInicial;

    int nps_perft;

    nps_perft = (double) lances_gerados / (double) delta;

    printf("Gerador de lances: %dK lances por segundo\n", nps_perft);
}

void Bench::search_performance_test(){
    Game::novo_jogo();

    Interface::profundidade_maxima = 11;
    Interface::tempo_maximo = 1 << 25;
    Interface::profundidade_fixa = true;
    Interface::tempo_fixo = false;

    int tempoInicial = Interface::obter_tempo();

    Interface::lance_computador(false);

    int delta = Interface::obter_tempo() - tempoInicial;

    int nps_search;

    nps_search = (double) lances_avaliados / (double) delta;

    printf("Pesquisa de lances: %dK posições por segundo\n", nps_search);
    printf("Tempo para chegar pesquisar na profundidade 11: %f segundos\n", (float) delta / 1000.0);
    printf("Lances avaliados: %dK\n", lances_avaliados / 1000);
    printf("Lance do computador: %s \n", Interface::lance_para_string(Game::lista_do_jogo[0].inicio, Game::lista_do_jogo[0].destino, 0));
}

void Bench::eval_performance_test(){
    int tempoInicial = Interface::obter_tempo();

    long avals = 0;

    while ((Interface::obter_tempo() - tempoInicial) < 5000){

        for (int i = 0; i < 4000; i++){
            Eval::avaliar();
        }
        
        avals += 4000;
    }

    int nps_eval = (double) avals / (double) 5000;
    
    printf("Avaliações de posições: %dK posições por segundo\n", nps_eval);
}