#include "gen.h"

#include <cstdlib>
#include <stdio.h>

#if defined(__BMI2__) && !defined(NOT_USE_PEXT)
    #include <immintrin.h>
    #define USE_PEXT
#else
    #define USE_MAGIC_HASHING
#endif

#include "search.h"
#include "game.h"
#include "update.h"
#include "interface.h"
#include "magics.h"

Bitboard::u64 peao_uma_casa[LADOS][CASAS_DO_TABULEIRO];
Bitboard::u64 peao_duas_casas[LADOS][CASAS_DO_TABULEIRO];

Bitboard::u64 bit_casas_relevantes_bispo[CASAS_DO_TABULEIRO];
Bitboard::u64 bit_casas_relevantes_torres[CASAS_DO_TABULEIRO];

Bitboard::u64 Gen::bit_esquerda[LADOS][CASAS_DO_TABULEIRO];
Bitboard::u64 Gen::bit_direita[LADOS][CASAS_DO_TABULEIRO];

Bitboard::u64 Gen::bit_peao_capturas[LADOS][CASAS_DO_TABULEIRO];
Bitboard::u64 Gen::bit_peao_defende[LADOS][CASAS_DO_TABULEIRO];

int Gen::peao_direita[LADOS][CASAS_DO_TABULEIRO];
int Gen::peao_esquerda[LADOS][CASAS_DO_TABULEIRO];

Bitboard::u64 Gen::bit_moves_cavalo[CASAS_DO_TABULEIRO];
Bitboard::u64 Gen::bit_moves_bispo[CASAS_DO_TABULEIRO];
Bitboard::u64 Gen::bit_moves_torre[CASAS_DO_TABULEIRO];
Bitboard::u64 Gen::bit_moves_dama[CASAS_DO_TABULEIRO];
Bitboard::u64 Gen::bit_moves_rei[CASAS_DO_TABULEIRO];

Bitboard::u64 Gen::bit_magicas_bispo[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];
Bitboard::u64 Gen::bit_magicas_torre[CASAS_DO_TABULEIRO][MAGIC_HASHTABLE_SIZE];

Gen::lance Gen::lista_de_lances[PILHA_DE_LANCES];

int dtb_moves[CASAS_DO_TABULEIRO][9];
int cavalo_moves[CASAS_DO_TABULEIRO][9];
int rei_moves[CASAS_DO_TABULEIRO][9];

int mc;

void init_peao_lookups(){
    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        Gen::peao_esquerda[BRANCAS][casa] = -1;
        Gen::peao_esquerda[PRETAS][casa] = -1;
        Gen::peao_direita[BRANCAS][casa] = -1;
        Gen::peao_direita[PRETAS][casa] = -1;
    
        if (Consts::colunas[casa] > COLUNA_A){
            if (Consts::linhas[casa] < COLUNA_H){
                Gen::peao_esquerda[BRANCAS][casa] = casa + 7;
                Bitboard::set_bit(Gen::bit_peao_capturas[BRANCAS][casa], Gen::peao_esquerda[BRANCAS][casa]);
                Bitboard::set_bit(Gen::bit_esquerda[BRANCAS][casa], Gen::peao_esquerda[BRANCAS][casa]);
            }
            if (Consts::linhas[casa] > COLUNA_A){
                Gen::peao_esquerda[PRETAS][casa] = casa - 9;
                Bitboard::set_bit(Gen::bit_peao_capturas[PRETAS][casa], Gen::peao_esquerda[PRETAS][casa]);
                Bitboard::set_bit(Gen::bit_esquerda[PRETAS][casa], Gen::peao_esquerda[PRETAS][casa]);
            }
        }

        if (Consts::colunas[casa] < COLUNA_H){
            if (Consts::linhas[casa] < COLUNA_H){
                Gen::peao_direita[BRANCAS][casa] = casa + 9;
                Bitboard::set_bit(Gen::bit_peao_capturas[BRANCAS][casa], Gen::peao_direita[BRANCAS][casa]);
                Bitboard::set_bit(Gen::bit_direita[BRANCAS][casa], Gen::peao_direita[BRANCAS][casa]);
            }
            if (Consts::linhas[casa] > COLUNA_A){
                Gen::peao_direita[PRETAS][casa] = casa - 7;
                Bitboard::set_bit(Gen::bit_peao_capturas[PRETAS][casa], Gen::peao_direita[PRETAS][casa]);
                Bitboard::set_bit(Gen::bit_direita[PRETAS][casa], Gen::peao_direita[PRETAS][casa]);
            }
        }

        Gen::bit_peao_defende[BRANCAS][casa] = Gen::bit_peao_capturas[PRETAS][casa];
        Gen::bit_peao_defende[PRETAS][casa] = Gen::bit_peao_capturas[BRANCAS][casa];

        if (Consts::linhas[casa] < COLUNA_H){
            peao_uma_casa[BRANCAS][casa] = casa + 8;
        }
        if (Consts::linhas[casa] < COLUNA_G){
            peao_duas_casas[BRANCAS][casa] = casa + 16;
        }
        if (Consts::linhas[casa] > COLUNA_A){ 
            peao_uma_casa[PRETAS][casa] = casa - 8;
        }
        if (Consts::linhas[casa] > COLUNA_B){
            peao_duas_casas[PRETAS][casa] = casa - 16;
        }

    }
}

void init_cavalo_lookups(){
    int qntt_lances, casa = 0;
    
    for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        qntt_lances = 0;

        if (Consts::linhas[casa] < 6 && Consts::colunas[casa] < COLUNA_H){
            cavalo_moves[casa][qntt_lances++] = casa + 17;
        }
        if (Consts::linhas[casa] < COLUNA_H && Consts::colunas[casa] < COLUNA_G){
            cavalo_moves[casa][qntt_lances++] = casa + 10;
        }
        if (Consts::linhas[casa] < 6 && Consts::colunas[casa] > COLUNA_A){
            cavalo_moves[casa][qntt_lances++] = casa + 15;
        }
        if (Consts::linhas[casa] < COLUNA_H && Consts::colunas[casa] > COLUNA_B){
            cavalo_moves[casa][qntt_lances++] = casa + 6;
        }
        if (Consts::linhas[casa] > 1 && Consts::colunas[casa] < COLUNA_H){
            cavalo_moves[casa][qntt_lances++] = casa - 15;
        }
        if (Consts::linhas[casa] > COLUNA_A && Consts::colunas[casa] < COLUNA_G){
            cavalo_moves[casa][qntt_lances++] = casa - 6;
        }
        if (Consts::linhas[casa] > 1 && Consts::colunas[casa] > COLUNA_A){
            cavalo_moves[casa][qntt_lances++] = casa - 17;
        }
        if (Consts::linhas[casa] > COLUNA_A && Consts::colunas[casa] > COLUNA_B){
            cavalo_moves[casa][qntt_lances++] = casa - 10;
        }

        cavalo_moves[casa][qntt_lances] = -1;
    }

    for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        qntt_lances = 0;
        while (cavalo_moves[casa][qntt_lances] > -1){
            Gen::bit_moves_cavalo[casa] |= Bitboard::mask[cavalo_moves[casa][qntt_lances]];
            qntt_lances++;
        }
    }
}

void init_rei_lookups(){
    int casa, qntt_lances, bit = 0;

    for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        bit = 0;
        qntt_lances = 0;

        if (Consts::colunas[casa] > COLUNA_A){
            rei_moves[casa][qntt_lances++] = casa-1;    
        }
        if (Consts::colunas[casa] < COLUNA_H){
            rei_moves[casa][qntt_lances++] = casa+1;    
        }
        if (Consts::linhas[casa] > COLUNA_A){
            rei_moves[casa][qntt_lances++] = casa-8;    
        }
        if (Consts::linhas[casa] < COLUNA_H){
            rei_moves[casa][qntt_lances++] = casa+8;    
        }
        if (Consts::colunas[casa] < COLUNA_H && Consts::linhas[casa] < COLUNA_H){
            rei_moves[casa][qntt_lances++] = casa+9;    
        }
        if (Consts::colunas[casa] > COLUNA_A && Consts::linhas[casa] < COLUNA_H){
            rei_moves[casa][qntt_lances++] = casa+7;    
        }
        if (Consts::colunas[casa] > COLUNA_A && Consts::linhas[casa] > COLUNA_A){
            rei_moves[casa][qntt_lances++] = casa-9;    
        }
        if (Consts::colunas[casa] < COLUNA_H && Consts::linhas[casa] > COLUNA_A){
            rei_moves[casa][qntt_lances++] = casa-7;    
        }

        rei_moves[casa][qntt_lances] = -1;

        while (rei_moves[casa][bit] > -1){
            Gen::bit_moves_rei[casa] |= Bitboard::mask[rei_moves[casa][bit]];
            bit++;
        }
    }   
}

void init_casas_relevantes(){
    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        bit_casas_relevantes_bispo[casa] = Gen::bit_moves_bispo[casa] & Bitboard::bordas_neg;
        bit_casas_relevantes_torres[casa] = Gen::bit_moves_torre[casa];

        if (Consts::colunas[casa] != COLUNA_A){
            bit_casas_relevantes_torres[casa] &= ~Bitboard::mask_cols[COLUNA_A];
        }
        if (Consts::colunas[casa] != COLUNA_H){
            bit_casas_relevantes_torres[casa] &= ~Bitboard::mask_cols[COLUNA_H];
        }
        if (Consts::linhas[casa] != PRIMEIRA_LINHA){
            bit_casas_relevantes_torres[casa] &= ~Bitboard::mask_rows[LINHA_1];
        }
        if (Consts::linhas[casa] != ULTIMA_LINHA){
            bit_casas_relevantes_torres[casa] &= ~Bitboard::mask_rows[LINHA_8];
        }
    }
}

void init_dtb_lookups(){
    int casa, qntt_lances = 0;

    for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        qntt_lances=0;

        for (int direcao = 0; direcao < 8; direcao++){
            dtb_moves[casa][direcao] = -1;
        }

        if (Consts::colunas[casa] > COLUNA_A){
            dtb_moves[casa][OESTE] = casa-1;
            rei_moves[casa][qntt_lances++] = casa-1;    
        }
        if (Consts::colunas[casa] < COLUNA_H){
            dtb_moves[casa][LESTE] = casa+1;
            rei_moves[casa][qntt_lances++] = casa+1;    
        }
        if (Consts::linhas[casa] > COLUNA_A){
            dtb_moves[casa][SUL] = casa-8;
            rei_moves[casa][qntt_lances++] = casa-8;    
        }
        if (Consts::linhas[casa] < COLUNA_H){
            dtb_moves[casa][NORTE] = casa+8;
            rei_moves[casa][qntt_lances++] = casa+8;    
        }
        if (Consts::colunas[casa] < COLUNA_H && Consts::linhas[casa] < COLUNA_H){
            dtb_moves[casa][NE] = casa+9;
            rei_moves[casa][qntt_lances++] = casa+9;    
        }
        if (Consts::colunas[casa] > COLUNA_A && Consts::linhas[casa] < COLUNA_H){
            dtb_moves[casa][NO] = casa+7;
            rei_moves[casa][qntt_lances++] = casa+7;    
        }
        if (Consts::colunas[casa] > COLUNA_A && Consts::linhas[casa] > COLUNA_A){
            dtb_moves[casa][SO] = casa-9;
            rei_moves[casa][qntt_lances++] = casa-9;    
        }
        if (Consts::colunas[casa] < COLUNA_H && Consts::linhas[casa] > COLUNA_A){
            dtb_moves[casa][SE] = casa-7;
            rei_moves[casa][qntt_lances++] = casa-7;    
        }

        rei_moves[casa][qntt_lances] = -1;
    }

    for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        Gen::bit_moves_bispo[casa] = 0;
        Gen::bit_moves_torre[casa] = 0;
        Gen::bit_moves_dama[casa] = 0;

        for (int casa2 = 0; casa2 < CASAS_DO_TABULEIRO; casa2++){            
            if (casa == casa2){
                continue;
            }

            if (Consts::no_diag[casa] == Consts::no_diag[casa2] || Consts::ne_diag[casa] == Consts::ne_diag[casa2]){
                Gen::bit_moves_bispo[casa] |= Bitboard::mask[casa2];
                Gen::bit_moves_dama[casa] |= Bitboard::mask[casa2];
            }

            if (Consts::colunas[casa] == Consts::colunas[casa2] || Consts::linhas[casa] == Consts::linhas[casa2]){
                Gen::bit_moves_torre[casa] |= Bitboard::mask[casa2];
                Gen::bit_moves_dama[casa] |= Bitboard::mask[casa2];
            }
        }
    }
}

Bitboard::u64 obterBloqueadoresPorCasa(int index, Bitboard::u64 raios){
    Bitboard::u64 bloqueadores = 0;

    int bloqueador = 0;
    int i = 0;

    while (raios){
        bloqueador = Bitboard::bitscan(raios);
        raios &= Bitboard::not_mask[bloqueador];

        if (index & (1 << i)){
            bloqueadores |= Bitboard::mask[bloqueador];
        }

        i++;
    }

    return bloqueadores;
}

Bitboard::u64 gerarLancesBispoSemMagica(int casa, Bitboard::u64 bloqueadores){
    Bitboard::u64 lances = 0;

    for (int direcao = NE; direcao < NO + 1; direcao += 2){ 
        for (int casa_destino = casa;;){
            casa_destino = dtb_moves[casa_destino][direcao];

            lances |= Bitboard::mask[casa_destino];
            
            if ((Bitboard::mask[casa_destino] & bloqueadores) || (casa_destino == -1)){
                break;
            }
        }
    }

    return lances;
}

Bitboard::u64 gerarLancesTorreSemMagica(int casa, Bitboard::u64 bloqueadores){
    Bitboard::u64 lances = 0;

    for (int direcao = NORTE; direcao < NO + 1; direcao += 2){
        for (int casa_destino = casa;;){
            casa_destino = dtb_moves[casa_destino][direcao];

            lances |= Bitboard::mask[casa_destino];

            if ((Bitboard::mask[casa_destino] & bloqueadores) || (casa_destino == -1)){
                break;
            }
        }
    }

    return lances;
}

void init_magic_lookups(){
    int index;
    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        for (int pecaBloqueadora = 0; pecaBloqueadora < (1 << (64-Magics::bits_indices_bispos[casa])); pecaBloqueadora++){
            Bitboard::u64 bloqueadores = obterBloqueadoresPorCasa(pecaBloqueadora, bit_casas_relevantes_bispo[casa]); 
          
            #ifdef USE_PEXT
                index = _pext_u64(bloqueadores, bit_casas_relevantes_bispo[casa]);
            #else
                index = (bloqueadores * Magics::magicas_bispos[casa]) >> (Magics::bits_indices_bispos[casa]);
            #endif

            Gen::bit_magicas_bispo[casa][index] = gerarLancesBispoSemMagica(casa, bloqueadores);   
        }

        for (int pecaBloqueadora = 0; pecaBloqueadora < (1 << (64-Magics::bits_indices_torres[casa])); pecaBloqueadora++){
            Bitboard::u64 bloqueadores = obterBloqueadoresPorCasa(pecaBloqueadora, bit_casas_relevantes_torres[casa]);
          
            #ifdef USE_PEXT
                index = _pext_u64(bloqueadores, bit_casas_relevantes_torres[casa]);
            #else
                index = (bloqueadores * Magics::magicas_torres[casa]) >> (Magics::bits_indices_torres[casa]);
            #endif

            Gen::bit_magicas_torre[casa][index] = gerarLancesTorreSemMagica(casa, bloqueadores);
        }
    }
}

void Gen::init_lookup_tables(){
    init_peao_lookups();
    init_cavalo_lookups();
    init_rei_lookups();
    init_dtb_lookups();
    init_casas_relevantes();
    init_magic_lookups();
}

int calcularBonusHeuristicas(const int origem, const int destino){
    Gen::lance contraLance = Search::contraLance_heuristica[Game::lista_do_jogo[Game::hply].inicio][Game::lista_do_jogo[Game::hply].destino];

    // Killer and counter-move tables store full lance values including the
    // promote piece. For non-promotion moves both sides carry 0, so the
    // equality stays consistent; for promotions it disambiguates variants.
    if (Search::killers_primarios[Game::ply].inicio == origem
        && Search::killers_primarios[Game::ply].destino == destino
        && Search::killers_primarios[Game::ply].promove == Gen::lista_de_lances[mc].promove){
        return SCORE_KILLER_1;
    }
    if (Search::killers_secundarios[Game::ply].inicio == origem
        && Search::killers_secundarios[Game::ply].destino == destino
        && Search::killers_secundarios[Game::ply].promove == Gen::lista_de_lances[mc].promove){
        return SCORE_KILLER_2;
    }
    if (contraLance.inicio == origem
        && contraLance.destino == destino
        && contraLance.promove == Gen::lista_de_lances[mc].promove){
        return SCORE_CONTRALANCE;
    }

    return Search::historico_heuristica[origem][destino];
}

void adicionar_captura(const int origem, const int destino, const int score){
    Gen::lista_de_lances[mc].inicio = origem;
    Gen::lista_de_lances[mc].destino = destino;
    Gen::lista_de_lances[mc].promove = 0;
    Gen::lista_de_lances[mc].score = score; // ordena por capturas
    mc++;
}

void adicionar_roque(const int origem, const int destino){
    Gen::lista_de_lances[mc].inicio = origem;
    Gen::lista_de_lances[mc].destino = destino;
    Gen::lista_de_lances[mc].promove = 0;
    Gen::lista_de_lances[mc].score = SCORE_ROQUE; // ordena por roque TODO TESTAR SE ESSA ORDENAÇÃO REALMENTE RESULTA EM MELHORAS ????????
    mc++;
}

void adicionar_lance(const int origem, const int destino){
    Gen::lista_de_lances[mc].inicio = origem;
    Gen::lista_de_lances[mc].destino = destino;
    Gen::lista_de_lances[mc].promove = 0;
    Gen::lista_de_lances[mc].score = calcularBonusHeuristicas(origem, destino);
    mc++;
}

// Emit Q + N promotion variants for a pawn reaching the back rank. Rook and
// bishop are skipped: R-promotion only matters in rare stalemate-avoidance
// positions, and B is dominated by Q in essentially every legal setup — the
// branching-factor cost of generating them outweighs the Elo they save.
// Captured-piece value is rolled into both scores so MVV/LVA is preserved.
void adicionar_promocao_variantes(const int origem, const int destino, const int piece_capturado){
    const int cap_bonus = (piece_capturado < VAZIO) ? Values::pieces_valor[piece_capturado] : 0;
    const bool is_capture = (piece_capturado < VAZIO);

    // Queen
    Gen::lista_de_lances[mc].inicio = origem;
    Gen::lista_de_lances[mc].destino = destino;
    Gen::lista_de_lances[mc].promove = D;
    Gen::lista_de_lances[mc].score = (is_capture ? SCORE_PROMO_Q_CAP : SCORE_PROMO_Q) + cap_bonus;
    mc++;

    // Knight
    Gen::lista_de_lances[mc].inicio = origem;
    Gen::lista_de_lances[mc].destino = destino;
    Gen::lista_de_lances[mc].promove = C;
    Gen::lista_de_lances[mc].score = (is_capture ? SCORE_PROMO_N_CAP : SCORE_PROMO_N) + cap_bonus;
    mc++;
}

// Queen-only capture-promotion for qsearch. Quiescence scores captures via
// pesquisa_quiescence(inicio, destino) and makes them with Update::fazer_captura,
// neither of which knows about the promote piece — so emitting N/R/B here just
// forces SEE to run 4× with identical inputs and identical results. Emit Q only.
void adicionar_promocao_captura_dama(const int origem, const int destino, const int piece_capturado){
    Gen::lista_de_lances[mc].inicio = origem;
    Gen::lista_de_lances[mc].destino = destino;
    Gen::lista_de_lances[mc].promove = D;
    Gen::lista_de_lances[mc].score = Values::px[piece_capturado];
    mc++;
}

void gerar_en_passant(){
    int ep = Game::lista_do_jogo[Game::hply - 1].destino;

    if (Bitboard::tabuleiro[ep] == P && abs(Game::lista_do_jogo[Game::hply - 1].inicio - ep) == AVANCO_DUPLO){
        if (Consts::colunas[ep] > COLUNA_A && Bitboard::mask[ep-1] & Bitboard::bit_pieces[Game::lado][P]){
            adicionar_captura(ep-1, peao_uma_casa[Game::lado][ep], EN_PASSANT_SCORE);
        }
        if (Consts::colunas[ep] < COLUNA_H && Bitboard::mask[ep+1] & Bitboard::bit_pieces[Game::lado][P]){
            adicionar_captura(ep+1, peao_uma_casa[Game::lado][ep], EN_PASSANT_SCORE);
        }
    }
}

void gerar_roques(){
    if (Game::lado == BRANCAS){
        if (Game::roque & BRANCAS_ROQUE_MENOR && !(Bitboard::bit_entre[H1][E1] & Bitboard::bit_total)){
            adicionar_roque(E1, G1);
        }
        if (Game::roque & BRANCAS_ROQUE_MAIOR && !(Bitboard::bit_entre[A1][E1] & Bitboard::bit_total)){
            adicionar_roque(E1, C1);
        }
    }
    else{
        if (Game::roque & PRETAS_ROQUE_MENOR && !(Bitboard::bit_entre[H8][E8] & Bitboard::bit_total)){
            adicionar_roque(E8, G8);
        }
        if (Game::roque & PRETAS_ROQUE_MAIOR && !(Bitboard::bit_entre[A8][E8] & Bitboard::bit_total)){
            adicionar_roque(E8, C8);
        }
    }
}

void Gen::gerar_lances(const int lado_a_mover, const int contraLado){
    int casa, casa_destino;

    Bitboard::u64 t1, t2, t3;

    Bitboard::u64 ataques_deslizantes;

    mc = Game::qntt_lances_totais[Game::ply];

    gerar_en_passant();
    gerar_roques();

    // 1. gera os lances de peao
    // 1.1 verifica quais casas estao disponiveis
    if (lado_a_mover == BRANCAS){
        t1 = Bitboard::bit_pieces[BRANCAS][P] & ((Bitboard::bit_lados[PRETAS] & Bitboard::not_coluna_h) >> 7);
        t2 = Bitboard::bit_pieces[BRANCAS][P] & ((Bitboard::bit_lados[PRETAS] & Bitboard::not_coluna_a) >> 9);
        t3 = Bitboard::bit_pieces[BRANCAS][P] & ~(Bitboard::bit_total>>8);
    }
    else{
        t1 = Bitboard::bit_pieces[PRETAS][P] & ((Bitboard::bit_lados[BRANCAS] & Bitboard::not_coluna_h) << 9);
        t2 = Bitboard::bit_pieces[PRETAS][P] & ((Bitboard::bit_lados[BRANCAS] & Bitboard::not_coluna_a) << 7);
        t3 = Bitboard::bit_pieces[PRETAS][P] & ~(Bitboard::bit_total<<8);
    }

    // 1.2 adiciona capturas de peao para a esquerda
    while (t1){
        casa = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa];
        casa_destino = Gen::peao_esquerda[lado_a_mover][casa];
        if (Consts::linhas[casa_destino] == FILEIRA_1 || Consts::linhas[casa_destino] == FILEIRA_8){
            adicionar_promocao_variantes(casa, casa_destino, Bitboard::tabuleiro[casa_destino]);
        }
        else{
            adicionar_captura(casa, casa_destino, Values::px[Bitboard::tabuleiro[casa_destino]]);
        }
    }

    // 1.3 adiciona capturas de peao para a direita
    while (t2){
        casa = Bitboard::bitscan(t2);
        t2 &= Bitboard::not_mask[casa];
        casa_destino = Gen::peao_direita[lado_a_mover][casa];
        if (Consts::linhas[casa_destino] == FILEIRA_1 || Consts::linhas[casa_destino] == FILEIRA_8){
            adicionar_promocao_variantes(casa, casa_destino, Bitboard::tabuleiro[casa_destino]);
        }
        else{
            adicionar_captura(casa, casa_destino, Values::px[Bitboard::tabuleiro[casa_destino]]);
        }
    }

    // 1.4 adiciona avanços de peao
    while (t3){
        casa = Bitboard::bitscan(t3);
        t3 &= Bitboard::not_mask[casa];
        casa_destino = peao_uma_casa[lado_a_mover][casa];
        if (Consts::linhas[casa_destino] == FILEIRA_1 || Consts::linhas[casa_destino] == FILEIRA_8){
            adicionar_promocao_variantes(casa, casa_destino, VAZIO);
        }
        else{
            adicionar_lance(casa, casa_destino);

            // 1.4.1 avanço duplo (a double push can never reach the back rank)
            if (Bitboard::fileiras[lado_a_mover][casa] == 1 && Bitboard::tabuleiro[peao_duas_casas[lado_a_mover][casa]] == VAZIO){
                adicionar_lance(casa, peao_duas_casas[lado_a_mover][casa]);
            }
        }
    }

    // 2. gera lances de cavalo
    t1 = Bitboard::bit_pieces[lado_a_mover][C];
    while (t1){
        casa = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa];

        // 2.1 gera capturas do cavalo
        t2 = Gen::bit_moves_cavalo[casa] & Bitboard::bit_lados[contraLado];
        while (t2){
            casa_destino = Bitboard::bitscan(t2);
            t2 &= Bitboard::not_mask[casa_destino];

            adicionar_captura(casa, casa_destino, Values::cx[Bitboard::tabuleiro[casa_destino]]);
        }

        // 2.2 gera lances de cavalo
        t2 = Gen::bit_moves_cavalo[casa] & ~Bitboard::bit_total;
        while(t2){
            casa_destino = Bitboard::bitscan(t2);
            t2 &= Bitboard::not_mask[casa_destino];

            adicionar_lance(casa, casa_destino);
        }
    }

    // 3. gera lances de bispo
    t1 = Bitboard::bit_pieces[lado_a_mover][B];
    while (t1){
        casa = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa];
        
        #ifdef USE_PEXT
            ataques_deslizantes = Gen::bit_magicas_bispo[casa][_pext_u64(Bitboard::bit_total, bit_casas_relevantes_bispo[casa])];
        #else
            ataques_deslizantes = Gen::bit_magicas_bispo[casa][((Bitboard::bit_total & bit_casas_relevantes_bispo[casa]) * Magics::magicas_bispos[casa]) >> (Magics::bits_indices_bispos[casa])];
        #endif


        while (ataques_deslizantes){
            casa_destino = Bitboard::bitscan(ataques_deslizantes);
            ataques_deslizantes &= Bitboard::not_mask[casa_destino];

            if (Bitboard::mask[casa_destino] & Bitboard::bit_total){
                if (Bitboard::mask[casa_destino] & Bitboard::bit_lados[contraLado]){
                    adicionar_captura(casa, casa_destino, Values::bx[Bitboard::tabuleiro[casa_destino]]);
                }
                
                continue;
            }

            adicionar_lance(casa, casa_destino);
        }
    }

    // 4. gera lances de torre
    t1 = Bitboard::bit_pieces[lado_a_mover][T];
    while (t1){
        casa = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa];

        #ifdef USE_PEXT
            ataques_deslizantes = Gen::bit_magicas_torre[casa][_pext_u64(Bitboard::bit_total, bit_casas_relevantes_torres[casa])];
        #else
            ataques_deslizantes = Gen::bit_magicas_torre[casa][((Bitboard::bit_total & bit_casas_relevantes_torres[casa]) * Magics::magicas_torres[casa]) >> (Magics::bits_indices_torres[casa])];
        #endif

        while (ataques_deslizantes){
            casa_destino = Bitboard::bitscan(ataques_deslizantes);
            ataques_deslizantes &= Bitboard::not_mask[casa_destino];

            if (Bitboard::mask[casa_destino] & Bitboard::bit_total){
                if (Bitboard::mask[casa_destino] & Bitboard::bit_lados[contraLado]){
                    adicionar_captura(casa, casa_destino, Values::tx[Bitboard::tabuleiro[casa_destino]]);
                }

                continue;
            }

            adicionar_lance(casa, casa_destino);
        }
    }

    // 5. gera lances de dama
    t1 = Bitboard::bit_pieces[lado_a_mover][D];
    while (t1){
        casa = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa];

        #ifdef USE_PEXT
            ataques_deslizantes = bit_magicas_bispo[casa][_pext_u64(Bitboard::bit_total, bit_casas_relevantes_bispo[casa])] | bit_magicas_torre[casa][_pext_u64(Bitboard::bit_total, bit_casas_relevantes_torres[casa])];
        #else
            ataques_deslizantes = Gen::bit_magicas_bispo[casa][((Bitboard::bit_total & bit_casas_relevantes_bispo[casa]) * Magics::magicas_bispos[casa]) >> (Magics::bits_indices_bispos[casa])] | Gen::bit_magicas_torre[casa][((Bitboard::bit_total & bit_casas_relevantes_torres[casa]) * Magics::magicas_torres[casa]) >> (Magics::bits_indices_torres[casa])];
        #endif

        while (ataques_deslizantes){
            casa_destino = Bitboard::bitscan(ataques_deslizantes);
            ataques_deslizantes &= Bitboard::not_mask[casa_destino];

            if (Bitboard::mask[casa_destino] & Bitboard::bit_total){
                if (Bitboard::mask[casa_destino] & Bitboard::bit_lados[contraLado]){
                    adicionar_captura(casa, casa_destino, Values::dx[Bitboard::tabuleiro[casa_destino]]);
                }

                continue;
            }

            adicionar_lance(casa, casa_destino);
        }
    }

    // 6. gera lances de rei
    casa = Bitboard::bitscan(Bitboard::bit_pieces[lado_a_mover][R]);

    // 6.1 gera capturas
    t1 = Gen::bit_moves_rei[casa] & Bitboard::bit_lados[contraLado];
    while (t1){
        casa_destino = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa_destino];

        adicionar_captura(casa, casa_destino, Values::rx[Bitboard::tabuleiro[casa_destino]]);
    }

    // 6.2 gera lances sem capturas
    t1 = Gen::bit_moves_rei[casa] & ~Bitboard::bit_total;
    while (t1){
        casa_destino = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa_destino];

        adicionar_lance(casa, casa_destino);
    }

    Game::qntt_lances_totais[Game::ply + 1] = mc;
}

void Gen::gerar_capturas(const int lado_a_mover, const int contraLado){
    int casa, casa_destino;
    Bitboard::u64 t1, t2;
    mc = Game::qntt_lances_totais[Game::ply];


    // 1. gera capturas de peao. Quiet pawn pushes to the back rank (quiet
    // promotions) are deliberately NOT generated here: quiescence uses
    // Update::fazer_captura which has no path for non-capture destinations,
    // and expanding the qsearch move surface is a separate follow-up.
    // 1.1 verifica quais casas estao disponiveis
    if (lado_a_mover == BRANCAS){
        t1 = Bitboard::bit_pieces[BRANCAS][P] & ((Bitboard::bit_lados[PRETAS] & Bitboard::not_coluna_h) >> 7);
        t2 = Bitboard::bit_pieces[BRANCAS][P] & ((Bitboard::bit_lados[PRETAS] & Bitboard::not_coluna_a) >> 9);
    }
    else{
        t1 = Bitboard::bit_pieces[PRETAS][P] & ((Bitboard::bit_lados[BRANCAS] & Bitboard::not_coluna_h) << 9);
        t2 = Bitboard::bit_pieces[PRETAS][P] & ((Bitboard::bit_lados[BRANCAS] & Bitboard::not_coluna_a) << 7);
    }

    // 1.2 adiciona capturas de peao para a esquerda
    while (t1){
        casa = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa];
        casa_destino = Gen::peao_esquerda[lado_a_mover][casa];
        if (Consts::linhas[casa_destino] == FILEIRA_1 || Consts::linhas[casa_destino] == FILEIRA_8){
            adicionar_promocao_captura_dama(casa, casa_destino, Bitboard::tabuleiro[casa_destino]);
        }
        else{
            adicionar_captura(casa, casa_destino, Values::px[Bitboard::tabuleiro[casa_destino]]);
        }
    }

    // 1.3 adiciona capturas de peao para a direita
    while (t2){
        casa = Bitboard::bitscan(t2);
        t2 &= Bitboard::not_mask[casa];
        casa_destino = Gen::peao_direita[lado_a_mover][casa];
        if (Consts::linhas[casa_destino] == FILEIRA_1 || Consts::linhas[casa_destino] == FILEIRA_8){
            adicionar_promocao_captura_dama(casa, casa_destino, Bitboard::tabuleiro[casa_destino]);
        }
        else{
            adicionar_captura(casa, casa_destino, Values::px[Bitboard::tabuleiro[casa_destino]]);
        }
    }

    // 2. gera capturas de cavalo
    t1 = Bitboard::bit_pieces[lado_a_mover][C];
    while (t1){
        casa = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa];

        // 2.1 gera capturas do cavalo
        t2 = Gen::bit_moves_cavalo[casa] & Bitboard::bit_lados[contraLado];
        while (t2){
            casa_destino = Bitboard::bitscan(t2);
            t2 &= Bitboard::not_mask[casa_destino];

            adicionar_captura(casa, casa_destino, Values::cx[Bitboard::tabuleiro[casa_destino]]);
        }
    }

    // 3. gera capturas de bispo
    t1 = Bitboard::bit_pieces[lado_a_mover][B];
    while (t1){
        casa = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa];

        t2 = Gen::bit_moves_bispo[casa] & Bitboard::bit_lados[contraLado];
        while (t2){
            casa_destino = Bitboard::bitscan(t2);
            t2 &= Bitboard::not_mask[casa_destino];

            if (!(Bitboard::bit_entre[casa][casa_destino] & Bitboard::bit_total)){
                adicionar_captura(casa, casa_destino, Values::bx[Bitboard::tabuleiro[casa_destino]]);
            }
        }
    }

    // 4. gera capturas de torre
    t1 = Bitboard::bit_pieces[lado_a_mover][T];
    while (t1){
        casa = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa];

        t2 = Gen::bit_moves_torre[casa] & Bitboard::bit_lados[contraLado];
        while (t2){
            casa_destino = Bitboard::bitscan(t2);
            t2 &= Bitboard::not_mask[casa_destino];

            if (!(Bitboard::bit_entre[casa][casa_destino] & Bitboard::bit_total)){
                adicionar_captura(casa, casa_destino, Values::tx[Bitboard::tabuleiro[casa_destino]]);
            }
        }
    }

    // 5. gera capturas de dama
    t1 = Bitboard::bit_pieces[lado_a_mover][D];
    while (t1){
        casa = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa];

        t2 = Gen::bit_moves_dama[casa] & Bitboard::bit_lados[contraLado];
        while (t2){
            casa_destino = Bitboard::bitscan(t2);
            t2 &= Bitboard::not_mask[casa_destino];

            if (!(Bitboard::bit_entre[casa][casa_destino] & Bitboard::bit_total)){
                adicionar_captura(casa, casa_destino, Values::dx[Bitboard::tabuleiro[casa_destino]]);
            }
        }
    }

    // 6. gera capturas de rei
    casa = Bitboard::bitscan(Bitboard::bit_pieces[lado_a_mover][R]);
    t1 = Gen::bit_moves_rei[casa] & Bitboard::bit_lados[contraLado];
    while (t1){
        casa_destino = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa_destino];

        adicionar_captura(casa, casa_destino, Values::rx[Bitboard::tabuleiro[casa_destino]]);
    }


    Game::qntt_lances_totais[Game::ply + 1] = mc;
}

unsigned long long Gen::perft_node(int profunidade){
    if (profunidade == 0){
        return 1ULL;
    }

    unsigned long long total = 0;

    gerar_lances(Game::lado, Game::xlado);

    for (int i = Game::qntt_lances_totais[Game::ply]; i < Game::qntt_lances_totais[Game::ply+1]; i++){
        if (!Update::fazer_lance(Gen::lista_de_lances[i].inicio, Gen::lista_de_lances[i].destino, Gen::lista_de_lances[i].promove)){
            continue;
        }
        total += perft_node(profunidade - 1);
        Update::desfaz_lance();
    }

    return total;
}

unsigned long long Gen::perft(int profunidade){
    if (profunidade == 0){
        return 1ULL;
    }

    unsigned long long total = 0;

    gerar_lances(Game::lado, Game::xlado);

    for (int i = Game::qntt_lances_totais[Game::ply]; i < Game::qntt_lances_totais[Game::ply+1]; i++){
        if (!Update::fazer_lance(Gen::lista_de_lances[i].inicio, Gen::lista_de_lances[i].destino, Gen::lista_de_lances[i].promove)){
            continue;
        }

        unsigned long long no_lances = perft_node(profunidade - 1);
        Update::desfaz_lance();

        printf("%s: %llu \n", Interface::lance_para_string(Gen::lista_de_lances[i].inicio, Gen::lista_de_lances[i].destino, Gen::lista_de_lances[i].promove), no_lances);
        total += no_lances;
    }

    return total;
}
