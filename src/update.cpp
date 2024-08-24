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

int casa_reversa[LADOS] = {-8,8};

Game::jogo* j;

void mover_piece(const int l, const int p, const int inicio, const int destino){
    Bitboard::bit_pieces[l][p] &= Bitboard::not_mask[inicio];
    Bitboard::bit_pieces[l][p] |= Bitboard::mask[destino];

    Bitboard::bit_lados[l] &= Bitboard::not_mask[inicio];
    Bitboard::bit_lados[l] |= Bitboard::mask[destino];

    Bitboard::bit_total = Bitboard::bit_lados[BRANCAS] | Bitboard::bit_lados[PRETAS];

    Bitboard::tabuleiro[destino] = p;
    Bitboard::tabuleiro[inicio] = VAZIO;

    Hash::adicionar_chave(l, p, inicio);
    Hash::adicionar_chave(l, p, destino);
}

void Update::remover_piece(const int l, const int p, const int casa){
    Hash::adicionar_chave(l, p, casa);

    Bitboard::tabuleiro[casa] = VAZIO;

    Bitboard::bit_lados[l] &= Bitboard::not_mask[casa];
    Bitboard::bit_pieces[l][p] &= Bitboard::not_mask[casa];
    Bitboard::bit_total = Bitboard::bit_lados[BRANCAS] | Bitboard::bit_lados[PRETAS];
}

void Update::desfaz_lance(){
    Game::lado ^= 1;
    Game::xlado ^= 1;
    Game::ply--;
    Game::hply--;

    Game::jogo* ultimo_lance = &Game::lista_do_jogo[Game::hply];
    int inicio = ultimo_lance->inicio;
    int destino = ultimo_lance->destino;

    Game::roque = ultimo_lance->roque;
    Game::cinquenta = ultimo_lance->cinquenta;

    // desfaz captura em passant
    if (Bitboard::tabuleiro[destino] == P && ultimo_lance->captura == VAZIO && Consts::colunas[inicio] != Consts::colunas[destino]){
        adicionar_piece(Game::xlado, P, destino + casa_reversa[Game::lado]);
    }

    // desfaz promoção
    if (ultimo_lance->promove == D){
        adicionar_piece(Game::lado, P, inicio);
        remover_piece(Game::lado, Bitboard::tabuleiro[destino], destino);
    }
    // outros lances
    else{
        mover_piece(Game::lado, Bitboard::tabuleiro[destino], destino, inicio);
    }

    // desfaz capturas
    if (ultimo_lance->captura != VAZIO){
        adicionar_piece(Game::xlado, ultimo_lance->captura, destino);
    }

    // desfaz roque movendo a torre
    if (abs(inicio - destino) == ROQUE && Bitboard::tabuleiro[inicio] == R){
        // roque menor brancas
        if (destino == G1){
            mover_piece(Game::lado, T, F1, H1);
        }
        // roque maior brancas
        else if (destino == C1){
            mover_piece(Game::lado, T, D1, A1);
        }
        // roque menor pretas
        else if (destino == G8){
            mover_piece(Game::lado, T, F8, H8);
        }
        // roque maior pretas
        else if (destino == C8){
            mover_piece(Game::lado, T, D8, A8);
        }
    }
}

bool Update::fazer_lance(const int inicio, const int destino){
    // 1. lida com o roque do rei, movendo também a torre
    if (abs(inicio - destino) == ROQUE && Bitboard::tabuleiro[inicio] == R){

        // 1.1 verifica se o rei está em xeque
        if (Attacks::casa_esta_sendo_atacada(Game::xlado, inicio)){
            return false;
        }

        // 1.2 verifica se as casas intermediarias estão atacadas

        // 1.2.1 roque menor brancas
        if (destino == G1){
            if (Attacks::casa_esta_sendo_atacada(Game::xlado, F1)){
                return false;
            }
            mover_piece(Game::lado, T, H1, F1);
        }
        // 1.2.2 roque maior brancas
        else if (destino == C1){
            if (Attacks::casa_esta_sendo_atacada(Game::xlado, D1)){
                return false;
            }
            mover_piece(Game::lado, T, A1, D1);
        }
        // 1.2.3 roque menor pretas
        else if (destino == G8){
            if (Attacks::casa_esta_sendo_atacada(Game::xlado, F8)){
                return false;
            }
            mover_piece(Game::lado, T, H8, F8);
        }
        // 1.2.4 roque maior pretas
        else if (destino == C8){
            if (Attacks::casa_esta_sendo_atacada(Game::xlado, D8)){
                return false;
            }
            mover_piece(Game::lado, T, A8, D8);
        }
    }

    // 2. atualiza variaveis e adiciona na lista do jogo
    j = &Game::lista_do_jogo[Game::hply];

    j->inicio = inicio;
    j->destino = destino;
    j->captura = Bitboard::tabuleiro[destino];
    j->roque = Game::roque;
    j->cinquenta = Game::cinquenta;
    j->hash = Hash::chaveAtual;
    j->lock = Hash::lockAtual;
    j->promove = P;
    
    Game::roque &= Consts::roque_mask[inicio] & Consts::roque_mask[destino];

    Game::ply++;
    Game::hply++;
    Game::cinquenta++;

    // 3. realiza o lance 
    if (Bitboard::tabuleiro[inicio] == P){
        Game::cinquenta = 0;
        // captura de peao en passant
        if (Bitboard::tabuleiro[destino] == VAZIO && Consts::colunas[inicio] != Consts::colunas[destino]){
            remover_piece(Game::xlado, P, destino + casa_reversa[Game::lado]);
            mover_piece(Game::lado, Bitboard::tabuleiro[inicio], inicio, destino);
        }
        // lances com promoções
        else if ((Consts::linhas[destino] == FILEIRA_1 || Consts::linhas[destino] == FILEIRA_8)){
            remover_piece(Game::lado, P, inicio);

            if (Bitboard::tabuleiro[destino] < VAZIO){
                remover_piece(Game::xlado, Bitboard::tabuleiro[destino], destino);
            }

            adicionar_piece(Game::lado, D, destino);
            j->promove = D;
        }
        // capturas
        else if (Bitboard::tabuleiro[destino] < VAZIO){
            remover_piece(Game::xlado, Bitboard::tabuleiro[destino], destino);
            mover_piece(Game::lado, Bitboard::tabuleiro[inicio], inicio, destino);
        }
        // lances sem promoções
        else{
            mover_piece(Game::lado, Bitboard::tabuleiro[inicio], inicio, destino);
        }
    }
    else{
        // capturas (reseta a contagem de 50 lances)
        if (Bitboard::tabuleiro[destino] < VAZIO){
            Game::cinquenta = 0;
            remover_piece(Game::xlado, Bitboard::tabuleiro[destino], destino);
        }

        mover_piece(Game::lado, Bitboard::tabuleiro[inicio], inicio, destino);
    }

    Game::lado ^= 1;
    Game::xlado ^= 1;

    // 4. verifica se o lance deixou o rei em xeque

    if (Attacks::casa_esta_sendo_atacada(Game::lado, Bitboard::bitscan(Bitboard::bit_pieces[Game::xlado][R]))){
        desfaz_lance();
        return false;
    }

    return true;
}

void Update::adicionar_piece(const int l, const int piece, const int casa){
    Bitboard::tabuleiro[casa] = piece;

    Hash::adicionar_chave(l, piece, casa);

    Bitboard::bit_lados[l] |= Bitboard::mask[casa];
    Bitboard::bit_pieces[l][piece] |= Bitboard::mask[casa];
    Bitboard::bit_total = Bitboard::bit_lados[BRANCAS] | Bitboard::bit_lados[PRETAS];
}

void Update::desfaz_captura(){
    Game::lado ^= 1;
    Game::xlado ^= 1;

    --Game::ply;
    --Game::hply;

    mover_piece(Game::lado, Bitboard::tabuleiro[Game::lista_do_jogo[Game::hply].destino], Game::lista_do_jogo[Game::hply].destino, Game::lista_do_jogo[Game::hply].inicio);
    adicionar_piece(Game::xlado, Game::lista_do_jogo[Game::hply].captura, Game::lista_do_jogo[Game::hply].destino);
}

int Update::fazer_captura(const int inicio, const int destino){
    Game::lista_do_jogo[Game::hply].inicio = inicio;
    Game::lista_do_jogo[Game::hply].destino = destino;
    Game::lista_do_jogo[Game::hply].captura = Bitboard::tabuleiro[destino];

    ++Game::ply;
    ++Game::hply;
    
    remover_piece(Game::xlado, Bitboard::tabuleiro[destino], destino);
    mover_piece(Game::lado, Bitboard::tabuleiro[inicio], inicio, destino);

    Game::lado ^= 1;
    Game::xlado ^= 1;

    if (Attacks::casa_esta_sendo_atacada(Game::lado, Bitboard::bitscan(Bitboard::bit_pieces[Game::xlado][R]))){
        desfaz_captura();
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

void Update::setar_posicao(char posicao[80], char lado_a_jogar[1], char roques[4], char casa_en_passant[2], char hm[4], char fm[4]){
    //1. LIMPA O TABULEIRO
    memset(Bitboard::bit_pieces, 0, sizeof(Bitboard::bit_pieces));
    memset(Bitboard::bit_lados, 0, sizeof(Bitboard::bit_lados));
    Bitboard::bit_total = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        Bitboard::tabuleiro[casa] = VAZIO;
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

        adicionar_piece(obter_cor(posicao[i]), obter_peca(posicao[i]), ((linha * 8) + coluna));
        
        coluna++;
    }


    //3. DEFINE O LADO A JOGAR
    if (lado_a_jogar[0] == 'w'){
        Game::lado = 0;
        Game::xlado = 1;
    }
    else{
        Game::lado = 1;
        Game::xlado = 0;
    }


    //4. DEFINE OS DIREITOS DE ROQUE
    Game::roque = 0;
    for (int j = 0; j < 4; j++){
        if (roques[j] == '\0'){
            break;
        }

        switch (roques[j]){
            case 'K':
                Game::roque |= BRANCAS_ROQUE_MENOR;
                break;
            case 'Q':
                Game::roque |= BRANCAS_ROQUE_MAIOR;
                break;
            case 'k':
                Game::roque |= PRETAS_ROQUE_MENOR;
                break;
            case 'q':
                Game::roque |= PRETAS_ROQUE_MAIOR;
                break;
            default:
                break;
        }
    }

    // 5. DEFINE A QUANTIDADE DE LANCES
    char* hmptr = hm;
    std::string hmstr = hmptr;

    Game::cinquenta = std::stoi(hmstr);

    Game::ply = 1;
    Game::hply = 1;

    // 6. DEFINE A CASA DE EN PASSANT
    if (casa_en_passant[0] != '-'){
        j = &Game::lista_do_jogo[Game::hply-1];

        j->inicio = obter_casa_inicio_por_en_passant(casa_en_passant);
        j->destino = obter_casa_destino_por_en_passant(casa_en_passant);
    }
}
