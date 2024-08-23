#include "init.h"

#include "bitboard.h"
#include "gen.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "interface.h"

void Init::init(){
    Bitboard::init_bits();
    Bitboard::init_board();
    Eval::init_eval_tables();
    Gen::init_lookup_tables();
    Bitboard::init_vetores();
    Hash::iniciar_hash();

    Game::lado_do_computador = VAZIO;
    Game::jogador[BRANCAS] = 0;
    Game::jogador[PRETAS] = 0;
    
    Interface::tempo_maximo = 1 << 25;
    Interface::profundidade_maxima = 4;

    Game::novo_jogo();
}
