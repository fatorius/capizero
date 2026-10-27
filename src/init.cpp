#include "init.h"

#include "bitboard.h"
#include "gen.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "interface.h"
#include "search.h"
#include "estado.h"

void Init::init(){
    Bitboard::init_bits();
    Bitboard::init_board(estado_principal);
    Eval::init_eval_tables();
    Gen::init_lookup_tables();
    Eval::init_king_zone();
    Bitboard::init_vetores();
    Hash::iniciar_hash();
    Search::init_lmr();

    
    Interface::tempo_maximo = 1 << 25;
    Interface::profundidade_maxima = 4;

    Game::novo_jogo(estado_principal);
}
