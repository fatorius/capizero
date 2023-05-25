#include <iostream>

#include "bitboard.h"
#include "gen.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "interface.h"
#include "init.h"

using namespace std;

int main(){
    cout<<"capizero "<<CAPIZERO_VERSION<<" por Hugo Souza"<<endl;
    cout<<"build "<<BUILDNO<<endl;

    #ifdef DEBUG_BUILD
    cout<<"Build compilada para debug"<<endl;
    #endif

    cout<<"compilado em "<<__TIMESTAMP__<<endl;
    cout<<"\n"<<endl;

    init();

    cout<<"Programa iniciado"<<endl;

    while (ler_comando()){
        if (lado_do_computador == lado){
            lance_computador();
        }
    }

    cout<<endl<<"encerrando programa"<<endl;

    liberar_memoria();

    return 0;
}