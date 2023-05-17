#ifndef GEN
#define GEN

#include "bitboard.h"

typedef struct
{
    int inicio;
    int destino;
    int promove;
    int score;
} lance;

extern lance lista_de_lances[PILHA_DE_LANCES];

extern u64 bit_moves_peao[LADOS][CASAS_DO_TABULEIRO];
extern u64 bit_moves_cavalo[CASAS_DO_TABULEIRO];
extern u64 bit_moves_bispo[CASAS_DO_TABULEIRO];
extern u64 bit_moves_torre[CASAS_DO_TABULEIRO];
extern u64 bit_moves_dama[CASAS_DO_TABULEIRO];
extern u64 bit_moves_rei[CASAS_DO_TABULEIRO];

extern u64 bit_peao_capturas[LADOS][CASAS_DO_TABULEIRO];
extern u64 bit_peao_defende[LADOS][CASAS_DO_TABULEIRO];

void init_lookup_tables();
void gerar_lances(const int lado_a_mover, const int contraLado);
void gerar_capturas(const int lado_a_mover, const int contraLado);

#endif