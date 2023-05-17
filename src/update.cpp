#include "update.h"

#include <cstdlib>

#include "bitboard.h"
#include "consts.h"
#include "hash.h"
#include "game.h"
#include "attacks.h"

int casa_reversa[2] = {-8,8};

jogo* j;

void mover_piece(const int l, const int p, const int inicio, const int destino){
    bit_pieces[l][p] &= not_mask[inicio];
    bit_pieces[l][p] |= mask[destino];

    bit_lados[l] &= not_mask[inicio];
    bit_lados[l] |= mask[destino];

    bit_total = bit_lados[BRANCAS] | bit_lados[PRETAS];

    tabuleiro[destino] = p;
    tabuleiro[inicio] = VAZIO;

    adicionar_chave(l, p, inicio);
    adicionar_chave(l, p, destino);
}

void remover_piece(const int l, const int p, const int casa){
    adicionar_chave(l, p, casa);

    tabuleiro[casa] = VAZIO;

    bit_lados[l] &= not_mask[casa];
    bit_pieces[l][p] &= not_mask[casa];
    bit_total = bit_lados[BRANCAS] | bit_lados[PRETAS];
}

void desfaz_lance(){
    lado ^= 1;
    xlado ^= 1;
    ply--;
    hply--;

    jogo* ultimo_lance = &lista_do_jogo[hply];
    int inicio = ultimo_lance->inicio;
    int destino = ultimo_lance->destino;

    roque = ultimo_lance->roque;
    cinquenta = ultimo_lance->cinquenta;

    // desfaz captura em passant
    if (tabuleiro[destino] == P && ultimo_lance->captura == VAZIO && colunas[inicio] != colunas[destino]){
        adicionar_piece(xlado, P, destino + casa_reversa[lado]);
    }

    // desfaz promoção
    if (ultimo_lance->promove == D){
        adicionar_piece(lado, P, inicio);
        remover_piece(lado, tabuleiro[destino], destino);
    }
    // outros lances
    else{
        mover_piece(lado, tabuleiro[destino], destino, inicio);
    }

    // desfaz capturas
    if (ultimo_lance->captura != VAZIO){
        adicionar_piece(xlado, ultimo_lance->captura, destino);
    }

    // desfaz roque movendo a torre
    if (abs(inicio - destino) == 2 && tabuleiro[inicio] == R){
        // roque menor brancas
        if (destino == G1){
            mover_piece(lado, T, F1, H1);
        }
        // roque maior brancas
        else if (destino == C1){
            mover_piece(lado, T, D1, A1);
        }
        // roque menor pretas
        else if (destino == G8){
            mover_piece(lado, T, F8, H8);
        }
        // roque maior pretas
        else if (destino == C8){
            mover_piece(lado, T, D8, A8);
        }
    }
}

bool fazer_lance(const int inicio, const int destino){

    // 1. lida com o roque do rei, movendo também a torre
    if (abs(inicio - destino) == 2 && tabuleiro[inicio] == R){

        // 1.1 verifica se o rei está em xeque
        if (casa_esta_sendo_atacada(xlado, inicio)){
            return false;
        }

        // 1.2 verifica se as casas intermediarias estão atacadas

        // 1.2.1 roque menor brancas
        if (destino == G1){
            if (casa_esta_sendo_atacada(xlado, F1)){
                return false;
            }
            mover_piece(lado, T, H1, F1);
        }
        // 1.2.2 roque maior brancas
        else if (destino == C1){
            if (casa_esta_sendo_atacada(xlado, D1)){
                return false;
            }
            mover_piece(lado, T, A1, D1);
        }
        // 1.2.3 roque menor pretas
        else if (destino == G8){
            if (casa_esta_sendo_atacada(xlado, G8)){
                return false;
            }
            mover_piece(lado, T, H8, F8);
        }
        // 1.2.4 roque maior pretas
        else if (destino == C8){
            if (casa_esta_sendo_atacada(xlado, D8)){
                return false;
            }
            mover_piece(lado, T, A8, D8);
        }
    }

    // 2. atualiza variaveis e adiciona na lista do jogo
    j = &lista_do_jogo[hply];

    j->inicio = inicio;
    j->destino = destino;
    j->captura = tabuleiro[destino];
    j->roque = roque;
    j->cinquenta = cinquenta;
    j->hash = chaveAtual;
    j->lock = lockAtual;
    
    roque &= roque_mask[inicio] & roque_mask[destino];

    ply++;
    hply++;
    cinquenta++;

    // 3. realiza o lance 

    if (tabuleiro[inicio] == P){
        cinquenta = 0;
        // captura de peao en passant
        if (tabuleiro[destino] == VAZIO && colunas[inicio] != colunas[destino]){
            remover_piece(xlado, P, destino + casa_reversa[lado]);
        }
    }

    // capturas (reseta a contagem de 50 lances)
    if (tabuleiro[destino] < VAZIO){
        cinquenta = 0;
        remover_piece(xlado, tabuleiro[destino], destino);
    }

    // lances com promoções
    if (tabuleiro[inicio] == P && (linhas[destino] == FILEIRA_1 || linhas[destino] == FILEIRA_8)){
        remover_piece(lado, P, inicio);
        adicionar_piece(lado, D, destino);
        j->promove = D;
    }
    // lances sem promoções
    else{
        j->promove = P;
        mover_piece(lado, tabuleiro[inicio], inicio, destino);
    }

    lado ^= 1;
    xlado ^= 1;

    // 4. verifica se o lance deixou o rei em xeque

    if (casa_esta_sendo_atacada(lado, bitscan(bit_pieces[xlado][R]))){
        desfaz_lance();
        return false;
    }

    return true;
}

void adicionar_piece(const int l, const int piece, const int casa){
    tabuleiro[casa] = piece;

    adicionar_chave(l, piece, casa);

    bit_lados[l] |= mask[casa];
    bit_pieces[l][piece] |= mask[casa];
    bit_total = bit_lados[BRANCAS] | bit_lados[PRETAS];
}
