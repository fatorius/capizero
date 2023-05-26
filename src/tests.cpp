#include <iostream>
#include <cstdlib>
#include <stdlib.h>

#include "tests.h"
#include "gen.h"
#include "interface.h"
#include "update.h"

using namespace std;


// Teste 1:
// Executa perft com profundidade 5 para verificar se
// a geração de lances está funcionando corretamente
void Tests::test1(){
    cout<<"Test 1 - Perft 5"<<endl;
    
    unsigned long long test1_valor_obtido = perft_node(5);

    cout<<"Valor esperado: "<<TEST1_VALOR_ESPERADO<<endl;
    cout<<"Valor obtido: "<<test1_valor_obtido<<endl;

    if (test1_valor_obtido == TEST1_VALOR_ESPERADO){
        TESTE_SUCESSO;
    }
    else{
        TESTE_FALHO;
    }
    cout<<"-----------------"<<endl;
}

// Teste 2:
// Executa uma pesquisa de lance pelo computador com o 
// limte de tempo de três segundos para verificar se o 
// a função int obter_tempo() e o algoritmo de checagem 
// do tempo estão funcionando corretamente
void Tests::test2(){
    cout<<"Test 2 - Pesquisa de lance com 3 segundos"<<endl;
    
    tempo_maximo = TEST2_VALOR_ESPERADO;
    profundidade_maxima = MAX_PLY;
    tempo_fixo = true;
    profundidade_fixa = false;
    
    lance_computador(false);

    int test2_valor_obtido = tempo_gasto;

    cout<<"Valor esperado: "<<TEST2_VALOR_ESPERADO<<"ms"<<endl;
    cout<<"Valor obtido: "<<test2_valor_obtido<<"ms"<<endl;
    cout<<"Margem de tolerância: "<<TEST2_TOLERANCIA<<"ms"<<endl;

    if (abs(test2_valor_obtido - TEST2_VALOR_ESPERADO) < TEST2_TOLERANCIA){
        TESTE_SUCESSO;
    }
    else{
        TESTE_FALHO;
    }

    cout<<"-----------------"<<endl;
}

// Teste 3
// Desfaz o último lance realizado pelo computador e 
// executa perft com profundidade 5 para verificar se 
// as funções de atualização do tabuleiro (fazer lance e
// desfazer lance) estão funcionando corretamente
void Tests::test3(){
    desfaz_lance();

    cout<<"Test 3 - Perft 5 após fazer e desfazer lance"<<endl;

    unsigned long long test1_valor_obtido = perft_node(5);

    cout<<"Valor esperado: "<<TEST1_VALOR_ESPERADO<<endl;
    cout<<"Valor obtido: "<<test1_valor_obtido<<endl;

    if (test1_valor_obtido == TEST1_VALOR_ESPERADO){
        TESTE_SUCESSO;
    }
    else{
        TESTE_FALHO;
    }
    cout<<"-----------------"<<endl;
}