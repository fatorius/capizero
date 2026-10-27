#include "update.h"

#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <string>


#include "bitboard.h"
#include "consts.h"
#include "hash.h"
#include "game.h"
#include "attacks.h"
#include "eval.h"
#include "estado.h"

int casa_reversa[LADOS] = {-8,8};

void mover_piece(Estado &est, const int l, const int p, const int inicio, const int destino){
    est.bit_pieces[l][p] &= Bitboard::not_mask[inicio];
    est.bit_pieces[l][p] |= Bitboard::mask[destino];

    est.bit_lados[l] &= Bitboard::not_mask[inicio];
    est.bit_lados[l] |= Bitboard::mask[destino];

    est.bit_total = est.bit_lados[BRANCAS] | est.bit_lados[PRETAS];

    est.tabuleiro[destino] = p;
    est.tabuleiro[inicio] = VAZIO;

    Hash::adicionar_chave(est, l, p, inicio);
    Hash::adicionar_chave(est, l, p, destino);
}

void Update::remover_piece(Estado &est, const int l, const int p, const int casa){
    Hash::adicionar_chave(est, l, p, casa);

    est.tabuleiro[casa] = VAZIO;

    est.bit_lados[l] &= Bitboard::not_mask[casa];
    est.bit_pieces[l][p] &= Bitboard::not_mask[casa];
    est.bit_total = est.bit_lados[BRANCAS] | est.bit_lados[PRETAS];

    est.fase_valor -= Eval::phase_weights[p];
}

void Update::desfaz_lance(Estado &est){
    est.lado ^= 1;
    est.xlado ^= 1;
    est.ply--;
    est.hply--;

    Game::jogo* ultimo_lance = &est.lista_do_jogo[est.hply];
    int inicio = ultimo_lance->inicio;
    int destino = ultimo_lance->destino;

    est.roque = ultimo_lance->roque;
    est.cinquenta = ultimo_lance->cinquenta;

    // desfaz captura em passant
    if (est.tabuleiro[destino] == P && ultimo_lance->captura == VAZIO && Consts::colunas[inicio] != Consts::colunas[destino]){
        adicionar_piece(est, est.xlado, P, destino + casa_reversa[est.lado]);
    }

    // desfaz promoção (any of C, B, T, D)
    if (ultimo_lance->promove >= C && ultimo_lance->promove <= D){
        adicionar_piece(est, est.lado, P, inicio);
        remover_piece(est, est.lado, est.tabuleiro[destino], destino);
    }
    // outros lances
    else{
        mover_piece(est, est.lado, est.tabuleiro[destino], destino, inicio);
    }

    // desfaz capturas
    if (ultimo_lance->captura != VAZIO){
        adicionar_piece(est, est.xlado, ultimo_lance->captura, destino);
    }

    // desfaz roque movendo a torre
    if (abs(inicio - destino) == ROQUE && est.tabuleiro[inicio] == R){
        // roque menor brancas
        if (destino == G1){
            mover_piece(est, est.lado, T, F1, H1);
        }
        // roque maior brancas
        else if (destino == C1){
            mover_piece(est, est.lado, T, D1, A1);
        }
        // roque menor pretas
        else if (destino == G8){
            mover_piece(est, est.lado, T, F8, H8);
        }
        // roque maior pretas
        else if (destino == C8){
            mover_piece(est, est.lado, T, D8, A8);
        }
    }
}

bool Update::fazer_lance(Estado &est, const int inicio, const int destino, const int promove){
    if (!(est.bit_lados[est.lado] & Bitboard::mask[inicio])) return false;
    if (est.bit_lados[est.lado] & Bitboard::mask[destino])  return false;
    
    // 1. lida com o roque do rei, movendo também a torre
    if (abs(inicio - destino) == ROQUE && est.tabuleiro[inicio] == R){

        // 1.1 verifica se o rei está em xeque
        if (Attacks::casa_esta_sendo_atacada(est, est.xlado, inicio)){
            return false;
        }

        // 1.2 verifica se as casas intermediarias estão atacadas

        // 1.2.1 roque menor brancas
        if (destino == G1){
            if (Attacks::casa_esta_sendo_atacada(est, est.xlado, F1)){
                return false;
            }
            mover_piece(est, est.lado, T, H1, F1);
        }
        // 1.2.2 roque maior brancas
        else if (destino == C1){
            if (Attacks::casa_esta_sendo_atacada(est, est.xlado, D1)){
                return false;
            }
            mover_piece(est, est.lado, T, A1, D1);
        }
        // 1.2.3 roque menor pretas
        else if (destino == G8){
            if (Attacks::casa_esta_sendo_atacada(est, est.xlado, F8)){
                return false;
            }
            mover_piece(est, est.lado, T, H8, F8);
        }
        // 1.2.4 roque maior pretas
        else if (destino == C8){
            if (Attacks::casa_esta_sendo_atacada(est, est.xlado, D8)){
                return false;
            }
            mover_piece(est, est.lado, T, A8, D8);
        }
    }

    // 2. atualiza variaveis e adiciona na lista do jogo
    Game::jogo *j = &est.lista_do_jogo[est.hply];

    j->inicio = inicio;
    j->destino = destino;
    j->captura = est.tabuleiro[destino];
    j->roque = est.roque;
    j->cinquenta = est.cinquenta;
    j->hash = est.chaveAtual;
    j->lock = est.lockAtual;
    j->promove = P;
    
    est.roque &= Consts::roque_mask[inicio] & Consts::roque_mask[destino];

    est.ply++;
    est.hply++;
    est.cinquenta++;

    // 3. realiza o lance 
    if (est.tabuleiro[inicio] == P){
        est.cinquenta = 0;
        // captura de peao en passant
        if (est.tabuleiro[destino] == VAZIO && Consts::colunas[inicio] != Consts::colunas[destino]){
            remover_piece(est, est.xlado, P, destino + casa_reversa[est.lado]);
            mover_piece(est, est.lado, est.tabuleiro[inicio], inicio, destino);
        }
        // lances com promoções
        else if ((Consts::linhas[destino] == FILEIRA_1 || Consts::linhas[destino] == FILEIRA_8)){
            remover_piece(est, est.lado, P, inicio);

            if (est.tabuleiro[destino] < VAZIO){
                remover_piece(est, est.xlado, est.tabuleiro[destino], destino);
            }

            const int promo_piece = (promove >= C && promove <= D) ? promove : D;
            adicionar_piece(est, est.lado, promo_piece, destino);
            j->promove = promo_piece;
        }
        // capturas
        else if (est.tabuleiro[destino] < VAZIO){
            remover_piece(est, est.xlado, est.tabuleiro[destino], destino);
            mover_piece(est, est.lado, est.tabuleiro[inicio], inicio, destino);
        }
        // lances sem promoções
        else{
            mover_piece(est, est.lado, est.tabuleiro[inicio], inicio, destino);
        }
    }
    else{
        // capturas (reseta a contagem de 50 lances)
        if (est.tabuleiro[destino] < VAZIO){
            est.cinquenta = 0;
            remover_piece(est, est.xlado, est.tabuleiro[destino], destino);
        }

        mover_piece(est, est.lado, est.tabuleiro[inicio], inicio, destino);
    }

    est.lado ^= 1;
    est.xlado ^= 1;

    // 4. verifica se o lance deixou o rei em xeque

    if (Attacks::casa_esta_sendo_atacada(est, est.lado, Bitboard::bitscan(est.bit_pieces[est.xlado][R]))){
        desfaz_lance(est);
        return false;
    }

    return true;
}

void Update::adicionar_piece(Estado &est, const int l, const int piece, const int casa){
    est.tabuleiro[casa] = piece;

    Hash::adicionar_chave(est, l, piece, casa);

    est.bit_lados[l] |= Bitboard::mask[casa];
    est.bit_pieces[l][piece] |= Bitboard::mask[casa];
    est.bit_total = est.bit_lados[BRANCAS] | est.bit_lados[PRETAS];

    est.fase_valor += Eval::phase_weights[piece];
}

void Update::desfaz_captura(Estado &est){
    est.lado ^= 1;
    est.xlado ^= 1;

    --est.ply;
    --est.hply;

    mover_piece(est, est.lado, est.tabuleiro[est.lista_do_jogo[est.hply].destino], est.lista_do_jogo[est.hply].destino, est.lista_do_jogo[est.hply].inicio);
    adicionar_piece(est, est.xlado, est.lista_do_jogo[est.hply].captura, est.lista_do_jogo[est.hply].destino);
}

void Update::fazer_null_move(Estado &est){
    // Push a sentinel history entry. EP availability in `gerar_en_passant`
    // is detected by reading the previous entry and checking for a double
    // pawn push — inicio == destino == 0 satisfies neither condition, so EP
    // gets correctly cleared for the side that gains the tempo.
    Game::jogo *j = &est.lista_do_jogo[est.hply];
    j->inicio    = 0;
    j->destino   = 0;
    j->promove   = P;
    j->captura   = VAZIO;
    j->cinquenta = est.cinquenta;
    j->roque     = est.roque;
    j->hash      = est.chaveAtual;
    j->lock      = est.lockAtual;

    // Seed the child ply's move-list start. Real moves go through
    // gerar_lances at the parent before any child enters, so qntt[ply+1] is
    // always written by the parent. Null-move skips parent's gen entirely,
    // so without this seed the child reads a stale value left by a previous
    // iteration and writes its moves to a region that may overlap another
    // active ply's list — corrupting moves that are about to be applied.
    est.qntt_lances_totais[est.ply + 1] = est.qntt_lances_totais[est.ply];

    est.ply++;
    est.hply++;
    est.cinquenta++;

    est.lado  ^= 1;
    est.xlado ^= 1;
    // Zobrist key intentionally not updated: capizero stores side-to-move
    // implicitly via per-side TT tables (`hashpos[BRANCAS]` vs `hashpos[PRETAS]`),
    // and the key has no castling/EP component to clear.
}

void Update::desfaz_null_move(Estado &est){
    est.lado  ^= 1;
    est.xlado ^= 1;
    est.ply--;
    est.hply--;

    Game::jogo *j = &est.lista_do_jogo[est.hply];
    est.cinquenta = j->cinquenta;
    est.roque     = j->roque;
}

int Update::fazer_captura(Estado &est, const int inicio, const int destino){
    est.lista_do_jogo[est.hply].inicio = inicio;
    est.lista_do_jogo[est.hply].destino = destino;
    est.lista_do_jogo[est.hply].captura = est.tabuleiro[destino];

    ++est.ply;
    ++est.hply;
    
    remover_piece(est, est.xlado, est.tabuleiro[destino], destino);
    mover_piece(est, est.lado, est.tabuleiro[inicio], inicio, destino);

    est.lado ^= 1;
    est.xlado ^= 1;

    if (Attacks::casa_esta_sendo_atacada(est, est.lado, Bitboard::bitscan(est.bit_pieces[est.xlado][R]))){
        desfaz_captura(est);
        return false;
    }

    return true;
}

int obter_cor(char p){
    switch (p){
        case 'P':
            return BRANCAS;
            break;
        case 'p':
            return PRETAS;
            break;
        case 'N':
            return BRANCAS;
            break;
        case 'n':
            return PRETAS;
            break;
        case 'B':
            return BRANCAS;
            break;
        case 'b':
            return PRETAS;
            break;
        case 'R':
            return BRANCAS;
            break;
        case 'r':
            return PRETAS;
            break;
        case 'Q':
            return BRANCAS;
            break;
        case 'q':
            return PRETAS;
            break;
        case 'K':
            return BRANCAS;
            break;
        case 'k':
            return PRETAS;
            break;
        default:
            return VAZIO;
            break;
    }
}

int obter_peca(char p){
    switch (p){
        case 'P':
            return P;
            break;
        case 'p':
            return P;
            break;
        case 'N':
            return C;
            break;
        case 'n':
            return C;
            break;
        case 'B':
            return B;
            break;
        case 'b':
            return B;
            break;
        case 'R':
            return T;
            break;
        case 'r':
            return T;
            break;
        case 'Q':
            return D;
            break;
        case 'q':
            return D;
            break;
        case 'K':
            return R;
            break;
        case 'k':
            return R;
            break;
        default:
            return VAZIO;
            break;
    }
}

int obter_casa_inicio_por_en_passant(char ep[2]){
    char* epptr = ep;
    std::string epstr = epptr;
    
    if (epstr == "a3"){
        return A2;
    }
    else if (epstr == "b3"){
        return B2;
    }
    else if (epstr == "c3"){
        return C2;
    }
    else if (epstr == "d3"){
        return D2;
    }
    else if (epstr == "e3"){
        return E2;
    }
    else if (epstr == "f3"){
        return F2;
    }
    else if (epstr == "g3"){
        return G2;
    }
    else if (epstr == "h3"){
        return H2;
    }
    else if (epstr == "a6"){
        return A7;
    }
    else if (epstr == "b6"){
        return B7;
    }
    else if (epstr == "c6"){
        return C7;
    }
    else if (epstr == "d6"){
        return D7;
    }
    else if (epstr == "e6"){
        return E7;
    }
    else if (epstr == "f6"){
        return F7;
    }
    else if (epstr == "g6"){
        return G7;
    }
    else{
        return H7;
    }
}

int obter_casa_destino_por_en_passant(char ep[2]){
    char* epptr = ep;
    std::string epstr = epptr;
    
    if (epstr == "a3"){
        return A4;
    }
    else if (epstr == "b3"){
        return B4;
    }
    else if (epstr == "c3"){
        return C4;
    }
    else if (epstr == "d3"){
        return D4;
    }
    else if (epstr == "e3"){
        return E4;
    }
    else if (epstr == "f3"){
        return F4;
    }
    else if (epstr == "g3"){
        return G4;
    }
    else if (epstr == "h3"){
        return H4;
    }
    else if (epstr == "a6"){
        return A5;
    }
    else if (epstr == "b6"){
        return B5;
    }
    else if (epstr == "c6"){
        return C5;
    }
    else if (epstr == "d6"){
        return D5;
    }
    else if (epstr == "e6"){
        return E5;
    }
    else if (epstr == "f6"){
        return F5;
    }
    else if (epstr == "g6"){
        return G5;
    }
    else{
        return H5;
    }
}

void Update::setar_posicao(Estado &est, char posicao[80], char lado_a_jogar[1], char roques[4], char casa_en_passant[2], char hm[4], char fm[4]){
    //1. LIMPA O TABULEIRO
    est.chaveAtual = 0;
    est.lockAtual  = 0;
    memset(est.bit_pieces, 0, sizeof(est.bit_pieces));
    memset(est.bit_lados, 0, sizeof(est.bit_lados));
    est.bit_total = 0;
    est.fase_valor = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        est.tabuleiro[casa] = VAZIO;
    }


    // 2. ADICIONA AS PEÇAS
    int coluna = 0;
    int linha = 7;

    for (int i = 0; i < 80; i++){
        if (posicao[i] == '\0'){
            break;
        }
        else if (posicao[i] == '/'){
            linha--;
            coluna = 0;
            continue;
        }
        else if (isdigit(posicao[i])){
            coluna += posicao[i] - '0';
            continue;
        }

        adicionar_piece(est, obter_cor(posicao[i]), obter_peca(posicao[i]), ((linha * 8) + coluna));
        
        coluna++;
    }


    //3. DEFINE O LADO A JOGAR
    if (lado_a_jogar[0] == 'w'){
        est.lado = 0;
        est.xlado = 1;
    }
    else{
        est.lado = 1;
        est.xlado = 0;
    }


    //4. DEFINE OS DIREITOS DE ROQUE
    est.roque = 0;
    for (int j = 0; j < 4; j++){
        if (roques[j] == '\0'){
            break;
        }

        switch (roques[j]){
            case 'K':
                est.roque |= BRANCAS_ROQUE_MENOR;
                break;
            case 'Q':
                est.roque |= BRANCAS_ROQUE_MAIOR;
                break;
            case 'k':
                est.roque |= PRETAS_ROQUE_MENOR;
                break;
            case 'q':
                est.roque |= PRETAS_ROQUE_MAIOR;
                break;
            default:
                break;
        }
    }

    // 5. DEFINE A QUANTIDADE DE LANCES
    char* hmptr = hm;
    std::string hmstr = hmptr;

    est.cinquenta = std::stoi(hmstr);

    est.ply = 1;
    est.hply = 1;

    // 6. DEFINE A CASA DE EN PASSANT
    Game::jogo *j = &est.lista_do_jogo[est.hply-1];
    memset(j, 0, sizeof(Game::jogo));
    
    if (casa_en_passant[0] != '-'){
        j->inicio = obter_casa_inicio_por_en_passant(casa_en_passant);
        j->destino = obter_casa_destino_por_en_passant(casa_en_passant);
    }
}
