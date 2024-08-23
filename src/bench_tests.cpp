#include <stdio.h>

#include "bitboard.h"
#include "gen.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "interface.h"
#include "init.h"
#include "bench.h"

int main(){
    printf("capizero %s por Hugo Souza 2023\n", CAPIZERO_VERSION);
    printf("build %d \n", BUILDNO);
    printf("compilado em %s \n\n", __TIMESTAMP__);
    printf("executando testes de performance \n\n\n");

    init();

    Bench::movegen_performance_test();
    Bench::eval_performance_test();
    Bench::search_performance_test();

    Hash::liberar_memoria();

    return 0;
}