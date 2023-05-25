#include <iostream>

#include "tests.h"
#include "gen.h"

using namespace std;

// Teste 1:
// Executa perft com profundidade 5 para verificar se
// a geração de lances está funcionando corretamente
void test1(){
    cout<<"-----------------"<<endl;
    cout<<"Test 1 - Perft 5"<<endl;
    
    unsigned long long test1_valor_obtido = perft_node(5);

    if (test1_valor_obtido == test1_valor_esperado){
        cout<<"TESTE BEM SUCEDIDO"<<endl;
    }
    else{
        cout<<"TESTE FALHO"<<endl;
    }

    cout<<"Valor esperado: "<<test1_valor_esperado<<endl;
    cout<<"Valor obtido: "<<test1_valor_obtido<<endl;
    cout<<"-----------------"<<endl;
}