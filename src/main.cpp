#include <iostream>

#include "main.h"

using namespace std;

#define __CAPIZERO__VERSION__ "0.1.0"

void init(){
    init_bits();
    init_board();
    init_eval_tables();
    init_lookup_tables();
}

int main(){
    cout<<"capizero "<<__CAPIZERO__VERSION__<<" por Hugo Souza"<<endl;

    init();

    cout<<"Programa iniciado"<<endl;

    return 0;
}