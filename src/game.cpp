#include "game.h"

#include "bitboard.h"
#include "gen.h"

int lado_do_computador;
int tempo_maximo;
int profundidade_maxima;

int jogador[2];

void novo_jogo(){
    init_board();
    gerar_lances(lado, xlado);
}