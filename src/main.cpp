#include <stdio.h>

#include "bitboard.h"
#include "gen.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "interface.h"
#include "init.h"

using namespace std;

int main(){
    printf("capizero %s por Hugo Souza 2023\n", CAPIZERO_VERSION);
    printf("build %d \n", BUILDNO);
    printf("compilado em %s \n", __TIMESTAMP__);

    #ifdef DEBUG_BUILD
    printf("Build compilada para debug\n");
    #endif

    printf("\n");

    Init::init();

    printf("\n\nalgoritmo iniciado\n\n");
    
    while (Interface::ler_comando()){
        if (Game::lado_do_computador == Game::lado){
            Interface::lance_computador(true);
        }
    }

    Hash::liberar_memoria();

    return 0;
}
