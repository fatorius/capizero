#include <iostream>

#include "main.h"

using namespace std;

#define __CAPIZERO__VERSION__ "0.1.0"

void init(){
    init_bits();
    init_board();
    init_eval_tables();
    init_lookup_tables();
    init_vetores();
    iniciar_hash();

    lado_do_computador = VAZIO;
    jogador[BRANCAS] = 0;
    jogador[PRETAS] = 0;
    tempo_maximo = 1 << 25;
    profundidade_maxima = 4;
}

int main(){
    cout<<"capizero "<<__CAPIZERO__VERSION__<<" por Hugo Souza"<<endl;

    init();

    cout<<"Programa iniciado"<<endl;

    novo_jogo();

    return 0;
}