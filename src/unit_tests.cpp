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
    cout<<"executando testes unitários do capizero \n"<<endl;

    init();

    Tests::test1();
    Tests::test2();
    Tests::test3();
    Tests::test4();

    liberar_memoria();

    return 0;
}