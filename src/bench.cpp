#include "bench.h"

#include <stdio.h>
#include <string.h>

#include "gen.h"
#include "interface.h"
#include "update.h"
#include "game.h"
#include "eval.h"
#include "search.h"
#include "hash.h"
#include "bench_fens.h"

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

    nps_search = (double) Search::lances_avaliados / (double) delta;

    printf("Pesquisa de lances: %dK posições por segundo\n", nps_search);
    printf("Tempo para chegar pesquisar na profundidade 11: %f segundos\n", (float) delta / 1000.0);
    printf("Lances avaliados: %dK\n", Search::lances_avaliados / 1000);
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

void Bench::run_standard_bench(int depth){
    unsigned long long total_nodes = 0;
    int total_ms = 0;

    Interface::profundidade_maxima = depth;
    Interface::tempo_maximo = 1 << 25;
    Interface::profundidade_fixa = true;
    Interface::tempo_fixo = false;

    for (int i = 0; i < Bench::bench_fens_count; i++){
        Hash::limpar_tt();

        char posicao[128], lado_a_jogar[8], roques[8], casa_en_passant[8], hm[8], fm[8];
        int n_read = sscanf(Bench::bench_fens[i], "%127s %7s %7s %7s %7s %7s",
            posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);

        if (n_read < 6){
            printf("info string bench: skipping malformed FEN #%d\n", i);
            continue;
        }

        Update::setar_posicao(posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);

        int t0 = Interface::obter_tempo();
        Search::pensar(false);
        int delta = Interface::obter_tempo() - t0;

        total_nodes += (unsigned long long) Search::lances_avaliados;
        total_ms += delta;

        printf("position %2d/%d: depth=%d nodes=%d time=%dms\n",
            i + 1, Bench::bench_fens_count, depth, Search::lances_avaliados, delta);
    }

    int nps_total = 0;
    if (total_ms > 0){
        nps_total = (int)((double) total_nodes * 1000.0 / (double) total_ms);
    }

    printf("\n===========================\n");
    printf("Total time (ms): %d\n", total_ms);
    printf("Nodes: %llu\n", total_nodes);
    printf("NPS: %d\n", nps_total);
}