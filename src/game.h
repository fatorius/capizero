#ifndef GAME
#define GAME

#include "bitboard.h"

struct Estado;

namespace Game{
    typedef struct{
        int inicio;
        int destino;
        int promove;
        int captura;
        int cinquenta;
        int roque;
        Bitboard::u64 hash;
        Bitboard::u64 lock;
    } jogo;

    void novo_jogo(Estado &est);
    void nova_posicao(Estado &est);
    bool checar_repeticoes(const Estado &est);
};

#endif