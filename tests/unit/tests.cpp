#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
#include <thread>

#include "tests.h"
#include "gen.h"
#include "interface.h"
#include "update.h"
#include "game.h"
#include "estado.h"
#include "search.h"
#include "hash.h"

using namespace std;


// Teste 1:
// Executa perft com profundidade 5 para verificar se
// a geração de lances está funcionando corretamente
void Tests::test1(){
    cout<<"Test 1 - Perft 5"<<endl;
    
    unsigned long long test1_valor_obtido = Gen::perft_node(estado_principal, 5);

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
    
    Interface::tempo_maximo = TEST2_VALOR_ESPERADO;
    Interface::profundidade_maxima = MAX_PLY;
    Interface::tempo_fixo = true;
    Interface::profundidade_fixa = false;

    Interface::lance_computador(estado_principal);

    int test2_valor_obtido = Interface::tempo_gasto;

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
    Game::novo_jogo(estado_principal);
    Update::fazer_lance(estado_principal, E2, E4);
    Update::desfaz_lance(estado_principal);

    cout<<"Test 3 - Perft 5 após fazer e desfazer lance"<<endl;

    unsigned long long test3_valor_obtido = Gen::perft_node(estado_principal, 5);

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

    Update::fazer_lance(estado_principal, E2, E4);
    Update::fazer_lance(estado_principal, E7, E5);
    Update::fazer_lance(estado_principal, G1, F3);
    Update::fazer_lance(estado_principal, B8, C6);

    Interface::tempo_maximo = 1000;
    Interface::profundidade_maxima = MAX_PLY;
    Interface::tempo_fixo = true;
    Interface::profundidade_fixa = false;

    Interface::lance_computador(estado_principal);

    Game::novo_jogo(estado_principal);

    unsigned long long test4_valor_obtido = Gen::perft_node(estado_principal, 5);

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
    Update::setar_posicao(estado_principal, posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);

    unsigned long long test5_valor_obtido = Gen::perft_node(estado_principal, 5);

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

    char posicao[80] = "rnb1k2r/pppp3p/7b/4Pp2/2B5/5QPq/PPPP1P1P/RNB1K2R";
    char lado_a_jogar[1] = {'w'};
    char roques[4] = {'K','Q','k','q'}; 
    char casa_en_passant[3] = "f6";
    char hm[4] = "0";
    char fm[4] = "11";
    Update::setar_posicao(estado_principal, posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);

    unsigned long long test6_valor_obtido = Gen::perft_node(estado_principal, 4);

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

static void setar_fen(Estado &est, const char *fen){
    char posicao[128], lado_a_jogar[8], roques[8], casa_en_passant[8], hm[8], fm[8];
    sscanf(fen, "%127s %7s %7s %7s %7s %7s", posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);
    Update::setar_posicao(est, posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);
}

static void perft_em_thread(Estado *est, const char *fen, int profundidade, unsigned long long *resultado){
    setar_fen(*est, fen);
    *resultado = Gen::perft_node(*est, profundidade);
}

// Teste 7
// Isolamento do Estado entre threads (pré-requisito do SMP):
// 7.1 dois perfts simultâneos, cada um com seu Estado e sua thread;
// 7.2 a mesma busca de profundidade fixa num Estado novo na thread principal
//     e noutro Estado novo numa segunda thread precisa visitar os mesmos nós.
//     Qualquer estado de busca que tenha ficado global vazaria da primeira
//     busca para a segunda e mudaria a contagem.
void Tests::test7(){
    cout<<"Test 7 - Isolamento do Estado entre threads"<<endl;

    const char *kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    const char *posicao3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";

    Estado *a = new Estado();
    Estado *b = new Estado();
    unsigned long long perft_a = 0, perft_b = 0;

    std::thread ta(perft_em_thread, a, kiwipete, 4, &perft_a);
    std::thread tb(perft_em_thread, b, posicao3, 5, &perft_b);
    ta.join();
    tb.join();

    cout<<"Perft 4 (kiwipete) esperado: "<<TEST7_PERFT_KIWIPETE<<" obtido: "<<perft_a<<endl;
    cout<<"Perft 5 (posição 3) esperado: "<<TEST7_PERFT_POSICAO3<<" obtido: "<<perft_b<<endl;

    delete a;
    delete b;

    Interface::profundidade_maxima = TEST7_PROFUNDIDADE_BUSCA;
    Interface::tempo_maximo = 1 << 25;
    Interface::profundidade_fixa = true;
    Interface::tempo_fixo = false;

    Estado *principal = new Estado();
    Hash::limpar_tt();
    setar_fen(*principal, kiwipete);
    Search::pensar(*principal, false);
    const int nos_principal = principal->lances_avaliados;
    delete principal;

    Estado *outro = new Estado();
    Hash::limpar_tt();
    setar_fen(*outro, kiwipete);
    std::thread tc(Search::pensar, std::ref(*outro), false, true);
    tc.join();
    const int nos_outro = outro->lances_avaliados;
    delete outro;

    cout<<"Busca profundidade "<<TEST7_PROFUNDIDADE_BUSCA<<": thread principal "<<nos_principal<<" nós, segunda thread "<<nos_outro<<" nós"<<endl;

    if (perft_a == TEST7_PERFT_KIWIPETE && perft_b == TEST7_PERFT_POSICAO3 && nos_principal == nos_outro){
        TESTE_SUCESSO;
    }
    else{
        TESTE_FALHO;
    }
    cout<<"-----------------"<<endl;
}

static bool entrada_igual(const Hash::EntradaTT &a, const Hash::EntradaTT &b){
    return a.inicio == b.inicio && a.destino == b.destino && a.score == b.score
        && a.depth == b.depth && a.bound == b.bound && a.promove == b.promove;
}

// Teste 8
// Entrada da TT de 16 bytes:
// 8.1 empacotar/desempacotar preserva todos os campos nos extremos;
// 8.2 uma entrada cuja metade foi trocada por outra escrita (leitura
//     "rasgada" entre threads) não passa na verificação do lock.
void Tests::test8(){
    cout<<"Test 8 - Entrada da TT (empacotamento e leitura rasgada)"<<endl;

    const int scores[] = {0, 1, -1, 37, -37, VALOR_XEQUE_MATE_BRANCAS, VALOR_XEQUE_MATE_PRETAS,
                          ALPHA_INICIAL, BETA_INICIAL, 2147483647, (-2147483647 - 1)};
    const int depths[] = {-128, -1, 0, 1, MAX_PLY, 127};
    const int casas[]  = {0, 27, 63};

    int falhas = 0;
    int casos = 0;
    for (unsigned si = 0; si < sizeof(scores) / sizeof(scores[0]); si++){
        for (unsigned di = 0; di < sizeof(depths) / sizeof(depths[0]); di++){
            for (unsigned ci = 0; ci < 3; ci++){
                for (int bound = 0; bound <= 3; bound++){
                    for (int promove = 0; promove <= 7; promove++){
                        Hash::EntradaTT e;
                        e.inicio  = casas[ci];
                        e.destino = casas[2 - ci];
                        e.score   = scores[si];
                        e.depth   = depths[di];
                        e.bound   = bound;
                        e.promove = promove;
                        if (!entrada_igual(e, Hash::desempacotar(Hash::empacotar(e)))){
                            falhas++;
                        }
                        casos++;
                    }
                }
            }
        }
    }
    cout<<"Empacotamento: "<<casos - falhas<<"/"<<casos<<" casos preservados"<<endl;

    Estado *est = new Estado();
    setar_fen(*est, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    est->ply = 0;
    Hash::limpar_tt();

    Gen::lance lc;
    lc.inicio = E2; lc.destino = A6; lc.promove = 0; lc._pad = 0; lc.score = 0;
    Hash::adicionar_hash(*est, est->lado, lc, -321, 9, TT_BOUND_EXACT, 0);

    const bool achou = Hash::hash_lookup(*est, est->lado)
        && est->hash_inicio == E2 && est->hash_destino == A6 && est->hash_score == -321
        && est->hash_depth == 9 && est->hash_bound == TT_BOUND_EXACT && est->hash_promove == 0;

    Hash::hashp *h = Hash::entrada(*est, est->lado);
    const Bitboard::u64 dados = h->dados.load();
    const Bitboard::u64 chave = h->chave_xor.load();

    // Metade "dados" de outra escrita, metade "chave" desta.
    h->dados.store(dados ^ (1ULL << 40));
    const bool dados_trocados = Hash::hash_lookup(*est, est->lado);
    h->dados.store(dados);

    // Metade "chave" de outra escrita, metade "dados" desta.
    h->chave_xor.store(chave ^ 0x9E3779B97F4A7C15ULL);
    const bool chave_trocada = Hash::hash_lookup(*est, est->lado);
    h->chave_xor.store(chave);

    const bool restaurada = Hash::hash_lookup(*est, est->lado);

    cout<<"Gravar e ler: "<<(achou ? "ok" : "FALHOU")
        <<" | dados trocados detectados: "<<(!dados_trocados ? "ok" : "FALHOU")
        <<" | chave trocada detectada: "<<(!chave_trocada ? "ok" : "FALHOU")
        <<" | entrada restaurada: "<<(restaurada ? "ok" : "FALHOU")<<endl;

    delete est;
    Hash::limpar_tt();

    if (falhas == 0 && achou && !dados_trocados && !chave_trocada && restaurada){
        TESTE_SUCESSO;
    }
    else{
        TESTE_FALHO;
    }
    cout<<"-----------------"<<endl;
}
