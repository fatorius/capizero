#ifndef GEN
#define GEN

#include "bitboard.h"
#include "params.h"

typedef struct
{
    int inicio;
    int destino;
    int promove;
    int score;
} lance;

extern lance lista_de_lances[PILHA_DE_LANCES];

extern u64 bit_esquerda[LADOS][CASAS_DO_TABULEIRO];
extern u64 bit_direita[LADOS][CASAS_DO_TABULEIRO];

extern int peao_direita[LADOS][CASAS_DO_TABULEIRO];
extern int peao_esquerda[LADOS][CASAS_DO_TABULEIRO];

extern u64 bit_moves_peao[LADOS][CASAS_DO_TABULEIRO];
extern u64 bit_moves_cavalo[CASAS_DO_TABULEIRO];
extern u64 bit_moves_bispo[CASAS_DO_TABULEIRO];
extern u64 bit_moves_torre[CASAS_DO_TABULEIRO];
extern u64 bit_moves_dama[CASAS_DO_TABULEIRO];
extern u64 bit_moves_rei[CASAS_DO_TABULEIRO];

extern u64 bit_magicas_bispo[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];
extern u64 bit_magicas_torre[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];

extern u64 bit_peao_capturas[LADOS][CASAS_DO_TABULEIRO];
extern u64 bit_peao_defende[LADOS][CASAS_DO_TABULEIRO];

void init_lookup_tables();
void gerar_lances(const int lado_a_mover, const int contraLado);
void gerar_capturas(const int lado_a_mover, const int contraLado);
unsigned long long perft_node(int profunidade);
unsigned long long perft(int profunidade);

#endif