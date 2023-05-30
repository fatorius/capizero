#include "game.h"

#include "gen.h"
#include "interface.h"

int lado_do_computador;

int jogador[2];

int lado;
int xlado;
int cinquenta;
int roque;
int ply;
int hply;
int primeiro_lance[MAX_PLY];
int jogada;


jogo lista_do_jogo[PILHA_DO_JOGO];

void novo_jogo(){
    lado = 0;
    xlado = 1;
    cinquenta = 0;
    roque = 15;
    ply = 0;
    hply = 0;
    primeiro_lance[0] = 0;
    jogada = 0;

    no_lances = 0;
    
    init_board();
    
    gerar_lances(lado, xlado);
}