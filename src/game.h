#ifndef GAME
#define GAME

#include "bitboard.h"

typedef struct
{
    int inicio;
    int destino;
    int promove;
    int captura;
    int cinquenta;
    int roque;
    u64 hash;
    u64 lock;
} jogo;

extern jogo lista_do_jogo[PILHA_DO_JOGO];

extern int lado_do_computador;
extern int tempo_maximo;
extern int profundidade_maxima;

extern int jogador[2];

extern int lado;
extern int xlado;
extern int cinquenta;
extern int roque;
extern int ply;
extern int hply;
extern int primeiro_lance[MAX_PLY];
extern int jogada;

void novo_jogo();

#endif