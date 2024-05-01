#include "init.h"

#include "bitboard.h"
#include "gen.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "interface.h"

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

    novo_jogo();
}
