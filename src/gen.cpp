#include "gen.h"

#include <cstdlib>
#include <stdio.h>

#include "search.h"
#include "game.h"
#include "update.h"
#include "interface.h"

u64 bit_esquerda[LADOS][CASAS_DO_TABULEIRO];
u64 bit_direita[LADOS][CASAS_DO_TABULEIRO];

u64 bit_peao_capturas[LADOS][CASAS_DO_TABULEIRO];
u64 bit_peao_defende[LADOS][CASAS_DO_TABULEIRO];

int peao_direita[LADOS][CASAS_DO_TABULEIRO];
int peao_esquerda[LADOS][CASAS_DO_TABULEIRO];

u64 peao_uma_casa[LADOS][CASAS_DO_TABULEIRO];
u64 peao_duas_casas[LADOS][CASAS_DO_TABULEIRO];

u64 bit_moves_cavalo[CASAS_DO_TABULEIRO];
u64 bit_moves_bispo[CASAS_DO_TABULEIRO];
u64 bit_moves_torre[CASAS_DO_TABULEIRO];
u64 bit_moves_dama[CASAS_DO_TABULEIRO];
u64 bit_moves_rei[CASAS_DO_TABULEIRO];

lance lista_de_lances[PILHA_DE_LANCES];

int dtb_moves[CASAS_DO_TABULEIRO][9];
int cavalo_moves[CASAS_DO_TABULEIRO][9];
int rei_moves[CASAS_DO_TABULEIRO][9];

int mc;

void init_peao_lookups(){
    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        peao_esquerda[BRANCAS][casa] = -1;
        peao_esquerda[PRETAS][casa] = -1;
        peao_direita[BRANCAS][casa] = -1;
        peao_direita[PRETAS][casa] = -1;
    
        if (colunas[casa] > COLUNA_A){
            if (linhas[casa] < COLUNA_H){
                peao_esquerda[BRANCAS][casa] = casa + 7;
                set_bit(bit_peao_capturas[BRANCAS][casa], peao_esquerda[BRANCAS][casa]);
                set_bit(bit_esquerda[BRANCAS][casa], peao_esquerda[BRANCAS][casa]);
            }
            if (linhas[casa] > COLUNA_A){
                peao_esquerda[PRETAS][casa] = casa - 9;
                set_bit(bit_peao_capturas[PRETAS][casa], peao_esquerda[PRETAS][casa]);
                set_bit(bit_esquerda[PRETAS][casa], peao_esquerda[PRETAS][casa]);
            }
        }

        if (colunas[casa] < COLUNA_H){
            if (linhas[casa] < COLUNA_H){
                peao_direita[BRANCAS][casa] = casa + 9;
                set_bit(bit_peao_capturas[BRANCAS][casa], peao_direita[BRANCAS][casa]);
                set_bit(bit_direita[BRANCAS][casa], peao_direita[BRANCAS][casa]);
            }
            if (linhas[casa] > COLUNA_A){
                peao_direita[PRETAS][casa] = casa - 7;
                set_bit(bit_peao_capturas[PRETAS][casa], peao_direita[PRETAS][casa]);
                set_bit(bit_direita[PRETAS][casa], peao_direita[PRETAS][casa]);
            }
        }

        bit_peao_defende[BRANCAS][casa] = bit_peao_capturas[PRETAS][casa];
        bit_peao_defende[PRETAS][casa] = bit_peao_capturas[BRANCAS][casa];

        if (linhas[casa] < COLUNA_H){
            peao_uma_casa[BRANCAS][casa] = casa + 8;
        }
        if (linhas[casa] < COLUNA_G){
            peao_duas_casas[BRANCAS][casa] = casa + 16;
        }
        if (linhas[casa] > COLUNA_A){ 
            peao_uma_casa[PRETAS][casa] = casa - 8;
        }
        if (linhas[casa] > COLUNA_B){
            peao_duas_casas[PRETAS][casa] = casa - 16;
        }

    }
}

void init_cavalo_lookups(){
    int qntt_lances, casa = 0;
    
    for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        qntt_lances = 0;

        if (linhas[casa] < 6 && colunas[casa] < COLUNA_H){
            cavalo_moves[casa][qntt_lances++] = casa + 17;
        }
        if (linhas[casa] < COLUNA_H && colunas[casa] < COLUNA_G){
            cavalo_moves[casa][qntt_lances++] = casa + 10;
        }
        if (linhas[casa] < 6 && colunas[casa] > COLUNA_A){
            cavalo_moves[casa][qntt_lances++] = casa + 15;
        }
        if (linhas[casa] < COLUNA_H && colunas[casa] > COLUNA_B){
            cavalo_moves[casa][qntt_lances++] = casa + 6;
        }
        if (linhas[casa] > 1 && colunas[casa] < COLUNA_H){
            cavalo_moves[casa][qntt_lances++] = casa - 15;
        }
        if (linhas[casa] > COLUNA_A && colunas[casa] < COLUNA_G){
            cavalo_moves[casa][qntt_lances++] = casa - 6;
        }
        if (linhas[casa] > 1 && colunas[casa] > COLUNA_A){
            cavalo_moves[casa][qntt_lances++] = casa - 17;
        }
        if (linhas[casa] > COLUNA_A && colunas[casa] > COLUNA_B){
            cavalo_moves[casa][qntt_lances++] = casa - 10;
        }

        cavalo_moves[casa][qntt_lances] = -1;
    }

    for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        qntt_lances = 0;
        while (cavalo_moves[casa][qntt_lances] > -1){
            bit_moves_cavalo[casa] |= mask[cavalo_moves[casa][qntt_lances]];
            qntt_lances++;
        }
    }
}

void init_rei_lookups(){
    int casa, qntt_lances, bit = 0;

    for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        bit = 0;
        qntt_lances = 0;

        if (colunas[casa] > COLUNA_A){
            rei_moves[casa][qntt_lances++] = casa-1;    
        }
        if (colunas[casa] < COLUNA_H){
            rei_moves[casa][qntt_lances++] = casa+1;    
        }
        if (linhas[casa] > COLUNA_A){
            rei_moves[casa][qntt_lances++] = casa-8;    
        }
        if (linhas[casa] < COLUNA_H){
            rei_moves[casa][qntt_lances++] = casa+8;    
        }
        if (colunas[casa] < COLUNA_H && linhas[casa] < COLUNA_H){
            rei_moves[casa][qntt_lances++] = casa+9;    
        }
        if (colunas[casa] > COLUNA_A && linhas[casa] < COLUNA_H){
            rei_moves[casa][qntt_lances++] = casa+7;    
        }
        if (colunas[casa] > COLUNA_A && linhas[casa] > COLUNA_A){
            rei_moves[casa][qntt_lances++] = casa-9;    
        }
        if (colunas[casa] < COLUNA_H && linhas[casa] > COLUNA_A){
            rei_moves[casa][qntt_lances++] = casa-7;    
        }

        rei_moves[casa][qntt_lances] = -1;

        while (rei_moves[casa][bit] > -1){
            bit_moves_rei[casa] |= mask[rei_moves[casa][bit]];
            bit++;
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

        if (colunas[casa] > COLUNA_A){
            dtb_moves[casa][OESTE] = casa-1;
            rei_moves[casa][qntt_lances++] = casa-1;    
        }
        if (colunas[casa] < COLUNA_H){
            dtb_moves[casa][LESTE] = casa+1;
            rei_moves[casa][qntt_lances++] = casa+1;    
        }
        if (linhas[casa] > COLUNA_A){
            dtb_moves[casa][SUL] = casa-8;
            rei_moves[casa][qntt_lances++] = casa-8;    
        }
        if (linhas[casa] < COLUNA_H){
            dtb_moves[casa][NORTE] = casa+8;
            rei_moves[casa][qntt_lances++] = casa+8;    
        }
        if (colunas[casa] < COLUNA_H && linhas[casa] < COLUNA_H){
            dtb_moves[casa][NE] = casa+9;
            rei_moves[casa][qntt_lances++] = casa+9;    
        }
        if (colunas[casa] > COLUNA_A && linhas[casa] < COLUNA_H){
            dtb_moves[casa][NO] = casa+7;
            rei_moves[casa][qntt_lances++] = casa+7;    
        }
        if (colunas[casa] > COLUNA_A && linhas[casa] > COLUNA_A){
            dtb_moves[casa][SO] = casa-9;
            rei_moves[casa][qntt_lances++] = casa-9;    
        }
        if (colunas[casa] < COLUNA_H && linhas[casa] > COLUNA_A){
            dtb_moves[casa][SE] = casa-7;
            rei_moves[casa][qntt_lances++] = casa-7;    
        }

        rei_moves[casa][qntt_lances] = -1;
    }

    for (casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        bit_moves_bispo[casa] = 0;
        bit_moves_torre[casa] = 0;
        bit_moves_dama[casa] = 0;

        for (int casa2 = 0; casa2 < CASAS_DO_TABULEIRO; casa2++){            
            if (casa == casa2){
                continue;
            }

            if (no_diag[casa] == no_diag[casa2] || ne_diag[casa] == ne_diag[casa2]){
                bit_moves_bispo[casa] |= mask[casa2];
                bit_moves_dama[casa] |= mask[casa2];
            }

            if (colunas[casa] == colunas[casa2] || linhas[casa] == linhas[casa2]){
                bit_moves_torre[casa] |= mask[casa2];
                bit_moves_dama[casa] |= mask[casa2];
            }
        }
    }
}

void init_lookup_tables(){
    init_peao_lookups();
    init_cavalo_lookups();
    init_rei_lookups();
    init_dtb_lookups();
}

int calcularBonusHeuristicas(const int origem, const int destino){
    lance contraLance = contraLance_heuristica[lista_do_jogo[hply].inicio][lista_do_jogo[hply].destino];

    if (contraLance.inicio == origem && contraLance.destino == destino){
        return SCORE_CONTRALANCE + historico_heuristica[origem][destino];
    }

    return historico_heuristica[origem][destino];
}

void adicionar_captura(const int origem, const int destino, const int score){
    lista_de_lances[mc].inicio = origem;
    lista_de_lances[mc].destino = destino;
    lista_de_lances[mc].score = score + SCORE_DE_CAPTURA; // ordena por capturas
    mc++;
}

void adicionar_roque(const int origem, const int destino){
    lista_de_lances[mc].inicio = origem;
    lista_de_lances[mc].destino = destino;
    lista_de_lances[mc].score = SCORE_ROQUE; // ordena por roque TODO TESTAR SE ESSA ORDENAÇÃO REALMENTE RESULTA EM MELHORAS ????????
    mc++;
}

void adicionar_lance(const int origem, const int destino){
    lista_de_lances[mc].inicio = origem;
    lista_de_lances[mc].destino = destino;
    lista_de_lances[mc].score = calcularBonusHeuristicas(origem, destino);
    mc++;
}

void gerar_en_passant(){
    int ep = lista_do_jogo[hply - 1].destino;

    if (tabuleiro[ep] == P && abs(lista_do_jogo[hply - 1].inicio - ep) == AVANCO_DUPLO){
        if (colunas[ep] > COLUNA_A && mask[ep-1] & bit_pieces[lado][P]){
            adicionar_captura(ep-1, peao_uma_casa[lado][ep], EN_PASSANT_SCORE);
        }
        if (colunas[ep] < COLUNA_H && mask[ep+1] & bit_pieces[lado][P]){
            adicionar_captura(ep+1, peao_uma_casa[lado][ep], EN_PASSANT_SCORE);
        }
    }
}

void gerar_roques(){
    if (lado == BRANCAS){
        if (roque & BRANCAS_ROQUE_MENOR && !(bit_entre[H1][E1] & bit_total)){
            adicionar_roque(E1, G1);
        }
        if (roque & BRANCAS_ROQUE_MAIOR && !(bit_entre[A1][E1] & bit_total)){
            adicionar_roque(E1, C1);
        }
    }
    else{
        if (roque & PRETAS_ROQUE_MENOR && !(bit_entre[H8][E8] & bit_total)){
            adicionar_roque(E8, G8);
        }
        if (roque & PRETAS_ROQUE_MAIOR && !(bit_entre[A8][E8] & bit_total)){
            adicionar_roque(E8, C8);
        }
    }
}

void gerar_lances(const int lado_a_mover, const int contraLado){
    int casa, direcao, casa_destino;

    u64 t1, t2, t3;

    mc = primeiro_lance[ply];

    gerar_en_passant();
    gerar_roques();

    // 1. gera os lances de peao
    // 1.1 verifica quais casas estao disponiveis
    if (lado_a_mover == BRANCAS){
        t1 = bit_pieces[BRANCAS][P] & ((bit_lados[PRETAS] & not_coluna_h) >> 7);
        t2 = bit_pieces[BRANCAS][P] & ((bit_lados[PRETAS] & not_coluna_a) >> 9);
        t3 = bit_pieces[BRANCAS][P] & ~(bit_total>>8);
    }
    else{
        t1 = bit_pieces[PRETAS][P] & ((bit_lados[BRANCAS] & not_coluna_h) << 9);
        t2 = bit_pieces[PRETAS][P] & ((bit_lados[BRANCAS] & not_coluna_a) << 7);
        t3 = bit_pieces[PRETAS][P] & ~(bit_total<<8);
    }

    // 1.2 adiciona capturas de peao para a esquerda
    while (t1){
        casa = bitscan(t1);
        t1 &= not_mask[casa];
        casa_destino = peao_esquerda[lado_a_mover][casa];
        adicionar_captura(casa, casa_destino, px[tabuleiro[casa_destino]]);
    }

    // 1.3 adiciona capturas de peao para a direita
    while (t2){
        casa = bitscan(t2);
        t2 &= not_mask[casa];
        casa_destino = peao_direita[lado_a_mover][casa];
        adicionar_captura(casa, casa_destino, px[tabuleiro[casa_destino]]);
    }
    
    // 1.4 adiciona avanços de peao
    while (t3){
        casa = bitscan(t3);
        t3 &= not_mask[casa];
        adicionar_lance(casa, peao_uma_casa[lado_a_mover][casa]);

        // 1.4.1 avanço duplo
        if (fileiras[lado_a_mover][casa] == 1 && tabuleiro[peao_duas_casas[lado_a_mover][casa]] == VAZIO){
            adicionar_lance(casa, peao_duas_casas[lado_a_mover][casa]);
        }
    }

    // 2. gera lances de cavalo
    t1 = bit_pieces[lado_a_mover][C];
    while (t1){
        casa = bitscan(t1);
        t1 &= not_mask[casa];

        // 2.1 gera capturas do cavalo
        t2 = bit_moves_cavalo[casa] & bit_lados[contraLado];
        while (t2){
            casa_destino = bitscan(t2);
            t2 &= not_mask[casa_destino];

            adicionar_captura(casa, casa_destino, cx[tabuleiro[casa_destino]]);
        }

        // 2.2 gera lances de cavalo
        t2 = bit_moves_cavalo[casa] & ~bit_total;
        while(t2){
            casa_destino = bitscan(t2);
            t2 &= not_mask[casa_destino];

            adicionar_lance(casa, casa_destino);
        }
    }

    // 3. gera lances de bispo
    t1 = bit_pieces[lado_a_mover][B];
    while (t1){
        casa = bitscan(t1);
        t1 &= not_mask[casa];

        for (direcao = NE; direcao < NO + 1; direcao += 2){ 
            // 3.1 gera diagonais com captura
            if (mask_vetores[casa][direcao] & bit_total){
                for (casa_destino = casa;;){
                    casa_destino = dtb_moves[casa_destino][direcao];

                    if (mask[casa_destino] & bit_total){
                        if (mask[casa_destino] & bit_lados[contraLado]){
                            adicionar_captura(casa, casa_destino, bx[tabuleiro[casa_destino]]);
                        }

                        break;
                    }

                    adicionar_lance(casa, casa_destino);
                }
            }
            // 3.2 gera diagonais sem captura
            else{
                for (casa_destino = casa;;){
                    casa_destino = dtb_moves[casa_destino][direcao];

                    if (casa_destino == -1){
                        break;
                    }

                    adicionar_lance(casa, casa_destino);
                }
            }
        }
    }

    // 4. gera lances de torre
    t1 = bit_pieces[lado_a_mover][T];
    while (t1){
        casa = bitscan(t1);
        t1 &= not_mask[casa];

        for (direcao = NORTE; direcao < NO + 1; direcao += 2){
            // 4.1 gera linhas com captura
            if (mask_vetores[casa][direcao] & bit_total){
                for (casa_destino = casa;;){
                    casa_destino = dtb_moves[casa_destino][direcao];

                    if (mask[casa_destino] & bit_total){
                        if (mask[casa_destino] & bit_lados[contraLado]){
                            adicionar_captura(casa, casa_destino, tx[tabuleiro[casa_destino]]);
                        }

                        break;
                    }

                    adicionar_lance(casa, casa_destino);
                }
            }
            // 4.2 gera linhas sem captura
            else{
                for (casa_destino = casa;;){
                    casa_destino = dtb_moves[casa_destino][direcao];

                    if (casa_destino == -1){
                        break;
                    }

                    adicionar_lance(casa, casa_destino);
                }
            }
        }
    }

    // 5. gera lances de dama
    t1 = bit_pieces[lado_a_mover][D];
    while (t1){
        casa = bitscan(t1);
        t1 &= not_mask[casa];

        for (direcao = NORTE; direcao < NO + 1; direcao++){
            // 5.1 gera linhas com captura
            if (mask_vetores[casa][direcao] & bit_total){
                for (casa_destino = casa;;){
                    casa_destino = dtb_moves[casa_destino][direcao];

                    if (mask[casa_destino] & bit_total){
                        if (mask[casa_destino] & bit_lados[contraLado]){
                            adicionar_captura(casa, casa_destino, dx[tabuleiro[casa_destino]]);
                        }

                        break;
                    }

                    adicionar_lance(casa, casa_destino);
                }
            }
            // 5.2 gera linhas sem captura
            else{
                for (casa_destino = casa;;){
                    casa_destino = dtb_moves[casa_destino][direcao];

                    if (casa_destino == -1){
                        break;
                    }

                    adicionar_lance(casa, casa_destino);
                }
            }
        }
    }

    // 6. gera lances de rei
    casa = bitscan(bit_pieces[lado_a_mover][R]);

    // 6.1 gera capturas
    t1 = bit_moves_rei[casa] & bit_lados[contraLado];
    while (t1){
        casa_destino = bitscan(t1);
        t1 &= not_mask[casa_destino];

        adicionar_captura(casa, casa_destino, rx[tabuleiro[casa_destino]]);
    }

    // 6.2 gera lances sem capturas
    t1 = bit_moves_rei[casa] & ~bit_total;
    while (t1){
        casa_destino = bitscan(t1);
        t1 &= not_mask[casa_destino];

        adicionar_lance(casa, casa_destino);
    }

    primeiro_lance[ply + 1] = mc;
}

void gerar_capturas(const int lado_a_mover, const int contraLado){
    int casa, casa_destino;
    u64 t1, t2;
    mc = primeiro_lance[ply];


    // 1. gera capturas de peao
    // 1.1 verifica quais casas estao disponiveis
    if (lado_a_mover == BRANCAS){
        t1 = bit_pieces[BRANCAS][P] & ((bit_lados[PRETAS] & not_coluna_h) >> 7);
        t2 = bit_pieces[BRANCAS][P] & ((bit_lados[PRETAS] & not_coluna_a) >> 9);
    }
    else{
        t1 = bit_pieces[PRETAS][P] & ((bit_lados[BRANCAS] & not_coluna_h) << 9);
        t2 = bit_pieces[PRETAS][P] & ((bit_lados[BRANCAS] & not_coluna_a) << 7);
    }

    // 1.2 adiciona capturas de peao para a esquerda
    while (t1){
        casa = bitscan(t1);
        t1 &= not_mask[casa];
        casa_destino = peao_esquerda[lado_a_mover][casa];
        adicionar_captura(casa, casa_destino, px[tabuleiro[casa_destino]]);
    }

    // 1.3 adiciona capturas de peao para a direita
    while (t2){
        casa = bitscan(t2);
        t2 &= not_mask[casa];
        casa_destino = peao_direita[lado_a_mover][casa];
        adicionar_captura(casa, casa_destino, px[tabuleiro[casa_destino]]);
    }
    
    // 2. gera capturas de cavalo
    t1 = bit_pieces[lado_a_mover][C];
    while (t1){
        casa = bitscan(t1);
        t1 &= not_mask[casa];

        // 2.1 gera capturas do cavalo
        t2 = bit_moves_cavalo[casa] & bit_lados[contraLado];
        while (t2){
            casa_destino = bitscan(t2);
            t2 &= not_mask[casa_destino];

            adicionar_captura(casa, casa_destino, cx[tabuleiro[casa_destino]]);
        }
    }

    // 3. gera capturas de bispo
    t1 = bit_pieces[lado_a_mover][B];
    while (t1){
        casa = bitscan(t1);
        t1 &= not_mask[casa];

        t2 = bit_moves_bispo[casa] & bit_lados[contraLado];
        while (t2){
            casa_destino = bitscan(t2);
            t2 &= not_mask[casa_destino];

            if (!(bit_entre[casa][casa_destino] & bit_total)){
                adicionar_captura(casa, casa_destino, bx[tabuleiro[casa_destino]]);
            }
        }
    }

    // 4. gera capturas de torre
    t1 = bit_pieces[lado_a_mover][T];
    while (t1){
        casa = bitscan(t1);
        t1 &= not_mask[casa];

        t2 = bit_moves_torre[casa] & bit_lados[contraLado];
        while (t2){
            casa_destino = bitscan(t2);
            t2 &= not_mask[casa_destino];

            if (!(bit_entre[casa][casa_destino] & bit_total)){
                adicionar_captura(casa, casa_destino, tx[tabuleiro[casa_destino]]);
            }
        }
    }

    // 5. gera capturas de dama
    t1 = bit_pieces[lado_a_mover][D];
    while (t1){
        casa = bitscan(t1);
        t1 &= not_mask[casa];

        t2 = bit_moves_dama[casa] & bit_lados[contraLado];
        while (t2){
            casa_destino = bitscan(t2);
            t2 &= not_mask[casa_destino];

            if (!(bit_entre[casa][casa_destino] & bit_total)){
                adicionar_captura(casa, casa_destino, dx[tabuleiro[casa_destino]]);
            }
        }
    }

    // 6. gera capturas de rei
    casa = bitscan(bit_pieces[lado_a_mover][R]);
    t1 = bit_moves_rei[casa] & bit_lados[contraLado];
    while (t1){
        casa_destino = bitscan(t1);
        t1 &= not_mask[casa_destino];

        adicionar_captura(casa, casa_destino, rx[tabuleiro[casa_destino]]);
    }


    primeiro_lance[ply + 1] = mc;
}

unsigned long long perft_node(int profunidade){
    if (profunidade == 0){
        return 1ULL;
    }

    unsigned long long total = 0;

    gerar_lances(lado, xlado);

    for (int i = primeiro_lance[ply]; i < primeiro_lance[ply+1]; i++){
        if (!fazer_lance(lista_de_lances[i].inicio, lista_de_lances[i].destino)){
            continue;
        }
        total += perft_node(profunidade - 1);
        desfaz_lance();
    }

    return total;
}

unsigned long long perft(int profunidade){
    if (profunidade == 0){
        return 1ULL;
    }

    unsigned long long total = 0;

    gerar_lances(lado, xlado);

    for (int i = primeiro_lance[ply]; i < primeiro_lance[ply+1]; i++){
        if (!fazer_lance(lista_de_lances[i].inicio, lista_de_lances[i].destino)){
            continue;
        }

        unsigned long long no_lances = perft_node(profunidade - 1);
        desfaz_lance();

        printf("%s: %llu \n", lance_para_string(lista_de_lances[i].inicio, lista_de_lances[i].destino, lista_de_lances[i].promove), no_lances);
        total += no_lances;
    }

    return total;
}