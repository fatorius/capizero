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
int qntt_lances_totais[MAX_PLY];
int jogada;


jogo lista_do_jogo[PILHA_DO_JOGO];

void novo_jogo(){
    lado = 0;
    xlado = 1;
    cinquenta = 0;
    roque = 15;
    ply = 0;
    hply = 1;
    qntt_lances_totais[0] = 0;
    jogada = 0;

    no_lances = 0;
    
    Bitboard::init_board();
    
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

    Eval::piece_mat[BRANCAS] = Eval::peao_mat[BRANCAS] = 0;
    Eval::piece_mat[PRETAS] = Eval::peao_mat[PRETAS] = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        if (Bitboard::tabuleiro[casa] < VAZIO){
            if (Bitboard::bit_lados[BRANCAS] & Bitboard::mask[casa]){
                c = BRANCAS;
            }
            else{
                c = PRETAS;
            }

            adicionar_piece(c, Bitboard::tabuleiro[casa], casa);
        }
    }

    chaveAtual = obter_chave();
    lockAtual = obter_lock();
}