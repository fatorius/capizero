#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bitboard.h"
#include "gen.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "interface.h"
#include "init.h"
#include "xboard.h"
#include "uci.h"
#include "bench.h"

using namespace std;

int main(int argc, char **argv){
    bool headless = (argc >= 2);

    if (!headless){
        printf("capizero %s por Hugo Souza 2023\n", CAPIZERO_VERSION);
        printf("build %d \n", BUILDNO);
        printf("compilado em %s \n", __TIMESTAMP__);

        #ifdef DEBUG_BUILD
        printf("Build compilada para debug\n");
        #endif

        printf("\n");
    }

    Init::init();

    if (headless){
        if (!strcmp(argv[1], "bench")){
            int depth = (argc >= 3) ? atoi(argv[2]) : 13;
            if (depth < 1){
                depth = 13;
            }
            Bench::run_standard_bench(depth);
            Hash::liberar_memoria();
            return 0;
        }
        if (!strcmp(argv[1], "uci")){
            Uci::loop();
            Hash::liberar_memoria();
            return 0;
        }
        if (!strcmp(argv[1], "xboard")){
            Interface::protocolo_ativo = Interface::PROTO_XBOARD;
            Xboard::xboard();
            Hash::liberar_memoria();
            return 0;
        }

        fprintf(stderr, "unknown argument: %s\n", argv[1]);
        fprintf(stderr, "usage: %s [bench [depth] | uci | xboard]\n", argv[0]);
        Hash::liberar_memoria();
        return 1;
    }

    printf("\n\nalgoritmo iniciado\n\n");

    while (Interface::ler_comando()){
        if (Game::lado_do_computador == Game::lado){
            Interface::lance_computador(true);
        }
    }

    Hash::liberar_memoria();

    return 0;
}
