#include <iostream>

#include "bitboard.h"
#include "gen.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "interface.h"
#include "tests.h"
#include "init.h"

using namespace std;

int main(){
    cout<<"executing capizero unit tests \n"<<endl;

    init();

    test1();

    liberar_memoria();

    return 0;
}