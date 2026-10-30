#ifndef BENCH
#define BENCH

namespace Bench{
    void movegen_performance_test();
    void search_performance_test();
    void eval_performance_test();
    void run_standard_bench(int depth);
    // ./capizero bench perf: perft 5, avaliações por segundo e busca até a
    // profundidade 11 a partir da posição inicial.
    void run_performance_bench();
}


#endif