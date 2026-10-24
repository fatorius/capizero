#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "init.h"
#include "hash.h"
#include "uci.h"
#include "bench.h"

int main(int argc, char **argv){
    Init::init();

    if (argc >= 2 && !strcmp(argv[1], "bench")){
        int depth = (argc >= 3) ? atoi(argv[2]) : 13;
        if (depth < 1){
            depth = 13;
        }
        Bench::run_standard_bench(depth);
        Hash::liberar_memoria();
        return 0;
    }

    if (argc >= 2 && strcmp(argv[1], "uci")){
        fprintf(stderr, "unknown argument: %s\n", argv[1]);
        fprintf(stderr, "usage: %s [uci | bench [depth]]\n", argv[0]);
        Hash::liberar_memoria();
        return 1;
    }

    Uci::loop();
    Hash::liberar_memoria();
    return 0;
}
