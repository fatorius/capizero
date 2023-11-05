#include <iostream>
#include <cstdlib>
#include <stdlib.h>

#include "tests.h"
#include "gen.h"
#include "interface.h"
#include "update.h"
#include "game.h"

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
    novo_jogo();
    fazer_lance(E2, E4);
    desfaz_lance();

    cout<<"Test 3 - Perft 5 após fazer e desfazer lance"<<endl;

    unsigned long long test3_valor_obtido = perft_node(5);

    cout<<"Valor esperado: "<<TEST3_VALOR_ESPERADO<<endl;
    cout<<"Valor obtido: "<<test3_valor_obtido<<endl;

    if (test3_valor_obtido == TEST3_VALOR_ESPERADO){
        TESTE_SUCESSO;
    }
    else{
        TESTE_FALHO;
    }
    cout<<"-----------------"<<endl;
}

// Teste 4
// Executa diversos lances e reinicia o estado do jogo,
// em seguida realiza um perft para verificar se a 
// função novo_jogo() está funcionando corretamente
void Tests::test4(){
    cout<<"Test 4 - Perft 5 após calcular lance e chamar novo_jogo()"<<endl;

    fazer_lance(E2, E4);
    fazer_lance(E7, E5);
    fazer_lance(G1, F3);
    fazer_lance(B8, C6);

    tempo_maximo = 1000;
    profundidade_maxima = MAX_PLY;
    tempo_fixo = true;
    profundidade_fixa = false;

    lance_computador(false);

    novo_jogo();

    unsigned long long test4_valor_obtido = perft_node(5);

    cout<<"Valor esperado: "<<TEST4_VALOR_ESPERADO<<endl;
    cout<<"Valor obtido: "<<test4_valor_obtido<<endl;

    if (test4_valor_obtido == TEST4_VALOR_ESPERADO){
        TESTE_SUCESSO;
    }
    else{
        TESTE_FALHO;
    }
    cout<<"-----------------"<<endl;
}

// Teste 5
// Define o tabuleiro para uma posição específica
// e executa perft 5 para verificar se a definição de
// posições no tabuleiro não altera algum estado do 
// jogo erroneamente
void Tests::test5(){
    // FEN = r1bqk2r/pp3ppp/2nppn2/2p5/2PP4/2PBPN2/P4PPP/R1BQK2R w KQkq - 0 1

    cout<<"Test 5 - Perft 5 após setar o tabuleiro em na posição r1bqk2r/pp3ppp/2nppn2/2p5/2PP4/2PBPN2/P4PPP/R1BQK2R w KQkq - 0 1"<<endl;

    char posicao[80] = "r1bqk2r/pp3ppp/2nppn2/2p5/2PP4/2PBPN2/P4PPP/R1BQK2R"; 
    char lado_a_jogar[1] = {'w'};
    char roques[4] = {'K','Q','k','q'}; 
    char casa_en_passant[2] = "-"; 
    char hm[4] = "0"; 
    char fm[4] = "1";
    setar_posicao(posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);

    unsigned long long test5_valor_obtido = perft_node(5);

    cout<<"Valor esperado: "<<TEST5_VALOR_ESPERADO<<endl;
    cout<<"Valor obtido: "<<test5_valor_obtido<<endl;

    if (test5_valor_obtido == TEST5_VALOR_ESPERADO){
        TESTE_SUCESSO;
    }
    else{
        TESTE_FALHO;
    }
    cout<<"-----------------"<<endl;
}

// Teste 6
// Semelhante ao teste anterior porém agora com uma captura 
// en passant disponível. 
// Verifica se a definição de tabuleiro manipula corretamente as casas de en passant
void Tests::test6(){
    //FEN = rnb1k2r/pppp3p/7b/4Pp2/2B5/5QPq/PPPP1P1P/RNB1K2R w KQkq f6 0 11

    cout<<"Test 6 - Perft 4 após setar o tabuleiro em na posição rnb1k2r/pppp3p/7b/4Pp2/2B5/5QPq/PPPP1P1P/RNB1K2R w KQkq f6 0 11"<<endl;

    char posicao[80] = "rrnb1k2r/pppp3p/7b/4Pp2/2B5/5QPq/PPPP1P1P/RNB1K2R"; 
    char lado_a_jogar[1] = {'w'};
    char roques[4] = {'K','Q','k','q'}; 
    char casa_en_passant[2] = {'f', '6'}; 
    char hm[4] = "0"; 
    char fm[4] = "11";
    setar_posicao(posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);

    unsigned long long test6_valor_obtido = perft_node(4);

    cout<<"Valor esperado: "<<TEST6_VALOR_ESPERADO<<endl;
    cout<<"Valor obtido: "<<test6_valor_obtido<<endl;

    if (test6_valor_obtido == TEST6_VALOR_ESPERADO){
        TESTE_SUCESSO;
    }
    else{
        TESTE_FALHO;
    }
    cout<<"-----------------"<<endl;
}

