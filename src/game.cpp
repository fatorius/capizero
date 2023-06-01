#include "game.h"

#include "gen.h"
#include "interface.h"
#include "eval.h"
#include "update.h"
#include "hash.h"

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

bool checar_repeticoes(){
    for (int i = hply-4; i >= hply - cinquenta; i -=2){
        if (lista_do_jogo[i].hash == chaveAtual && lista_do_jogo[i].lock == lockAtual){
            return true;
        }
    }
    return false;
}

void nova_posicao(){
    int c = 0;

    piece_mat[BRANCAS] = peao_mat[BRANCAS] = 0;
    piece_mat[PRETAS] = peao_mat[PRETAS] = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        if (tabuleiro[casa] < VAZIO){
            if (bit_lados[BRANCAS] & mask[casa]){
                c = BRANCAS;
            }
            else{
                c = PRETAS;
            }

            adicionar_piece(c, tabuleiro[casa], casa);
        }
    }

    chaveAtual = obter_chave();
    lockAtual = obter_lock();
}