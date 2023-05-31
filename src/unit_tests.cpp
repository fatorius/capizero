#include <stdio.h>

#include "bitboard.h"
#include "gen.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "interface.h"
#include "tests.h"
#include "init.h"

int main(){
    printf("capizero %s por Hugo Souza 2023\n", CAPIZERO_VERSION);
    printf("build %d \n", BUILDNO);
    printf("compilado em %s \n\n", __TIMESTAMP__);
    printf("executando testes unitários \n\n\n");

    init();

    Tests::test1();
    Tests::test2();
    Tests::test3();
    Tests::test4();

    liberar_memoria();

    return 0;
}