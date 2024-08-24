#ifndef GAME
#define GAME

#include "bitboard.h"

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

    extern jogo lista_do_jogo[PILHA_DO_JOGO];

    extern int lado_do_computador;

    extern int jogador[2];

    extern int lado;
    extern int xlado;
    extern int cinquenta;
    extern int roque;
    extern int ply;
    extern int hply;
    extern int qntt_lances_totais[MAX_PLY];
    extern int jogada;

    void novo_jogo();
    void nova_posicao();
    bool checar_repeticoes();
};

#endif