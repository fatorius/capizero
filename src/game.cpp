#include "game.h"

#include <string.h>

#include "gen.h"
#include "eval.h"
#include "update.h"
#include "hash.h"
#include "estado.h"

Estado estado_principal;

void Game::novo_jogo(Estado &est){
    est.lado = 0;
    est.xlado = 1;
    est.cinquenta = 0;
    est.roque = 15;
    est.ply = 0;
    est.hply = 1;
    est.qntt_lances_totais[0] = 0;
    est.jogada = 0;

    Bitboard::init_board(est);
    
    Gen::gerar_lances(est, est.lado, est.xlado);
}

bool Game::checar_repeticoes(const Estado &est){
    for (int i = est.hply-4; i >= est.hply - est.cinquenta && i >= 0; i -=2){
        if (est.lista_do_jogo[i].hash == est.chaveAtual && est.lista_do_jogo[i].lock == est.lockAtual){
            return true;
        }
    }
    return false;
}

void Game::nova_posicao(Estado &est){
    int c = 0;

    est.piece_mat[BRANCAS] = est.peao_mat[BRANCAS] = 0;
    est.piece_mat[PRETAS] = est.peao_mat[PRETAS] = 0;

    // Reset the incremental phase counter so the adicionar_piece loop
    // below rebuilds it from scratch (just like the material counters above).
    est.fase_valor = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        if (est.tabuleiro[casa] < VAZIO){
            if (est.bit_lados[BRANCAS] & Bitboard::mask[casa]){
                c = BRANCAS;
            }
            else{
                c = PRETAS;
            }

            Update::adicionar_piece(est, c, est.tabuleiro[casa], casa);
        }
    }

    est.chaveAtual = Hash::obter_chave(est);
    est.lockAtual = Hash::obter_lock(est);
}

void Game::copiar_posicao(Estado &dest, const Estado &orig){
    memcpy(dest.bit_pieces, orig.bit_pieces, sizeof(dest.bit_pieces));
    memcpy(dest.bit_lados, orig.bit_lados, sizeof(dest.bit_lados));
    dest.bit_total = orig.bit_total;
    memcpy(dest.tabuleiro, orig.tabuleiro, sizeof(dest.tabuleiro));

    dest.lado = orig.lado;
    dest.xlado = orig.xlado;
    dest.cinquenta = orig.cinquenta;
    dest.roque = orig.roque;
    dest.hply = orig.hply;
    dest.jogada = orig.jogada;
    dest.ply = 0;
    dest.qntt_lances_totais[0] = 0;

    dest.chaveAtual = orig.chaveAtual;
    dest.lockAtual = orig.lockAtual;
    memcpy(dest.peao_mat, orig.peao_mat, sizeof(dest.peao_mat));
    memcpy(dest.piece_mat, orig.piece_mat, sizeof(dest.piece_mat));
    dest.fase_valor = orig.fase_valor;

    // Entradas 0..hply: repetições e en passant olham para trás, e a ordenação
    // na raiz lê a entrada hply.
    memcpy(dest.lista_do_jogo, orig.lista_do_jogo, (orig.hply + 1) * sizeof(Game::jogo));
}