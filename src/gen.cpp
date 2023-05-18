#include "gen.h"

#include <cstdlib>

#include "search.h"
#include "game.h"

u64 bit_esquerda[LADOS][CASAS_DO_TABULEIRO];
u64 bit_direita[LADOS][CASAS_DO_TABULEIRO];

u64 bit_peao_capturas[LADOS][CASAS_DO_TABULEIRO];
u64 bit_peao_defende[LADOS][CASAS_DO_TABULEIRO];

u64 peao_direita[LADOS][CASAS_DO_TABULEIRO];
u64 peao_esquerda[LADOS][CASAS_DO_TABULEIRO];

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
    for (int x = 0; x < 64; x++){
        peao_esquerda[BRANCAS][x] = -1;
        peao_esquerda[PRETAS][x] = -1;
        peao_direita[BRANCAS][x] = -1;
        peao_direita[PRETAS][x] = -1;
    
        if (colunas[x] > COLUNA_A){
            if (linhas[x] < COLUNA_H){
                peao_esquerda[BRANCAS][x] = x + 7;
                set_bit(bit_peao_capturas[BRANCAS][x], peao_esquerda[BRANCAS][x]);
                set_bit(bit_esquerda[BRANCAS][x], peao_esquerda[BRANCAS][x]);
            }
            if (linhas[x] > COLUNA_A){
                peao_esquerda[PRETAS][x] = x - 9;
                set_bit(bit_peao_capturas[PRETAS][x], peao_esquerda[PRETAS][x]);
                set_bit(bit_esquerda[PRETAS][x], peao_esquerda[PRETAS][x]);
            }
        }

        if (colunas[x] < COLUNA_H){
            if (linhas[x] < COLUNA_H){
                peao_direita[BRANCAS][x] = x + 9;
                set_bit(bit_peao_capturas[BRANCAS][x], peao_esquerda[BRANCAS][x]);
                set_bit(bit_esquerda[BRANCAS][x], peao_esquerda[BRANCAS][x]);
            }
            if (linhas[x] > COLUNA_A){
                peao_direita[PRETAS][x] = x - 7;
                set_bit(bit_peao_capturas[PRETAS][x], peao_direita[PRETAS][x]);
                set_bit(bit_direita[PRETAS][x], peao_direita[PRETAS][x]);
            }
        }

        bit_peao_defende[BRANCAS][x] = bit_peao_capturas[PRETAS][x];
        bit_peao_defende[PRETAS][x] = bit_peao_capturas[BRANCAS][x];

        if (linhas[x] < COLUNA_H){
            peao_uma_casa[BRANCAS][x] = x + 8;
        }
        if (linhas[x] < COLUNA_G){
            peao_duas_casas[BRANCAS][x] = x + 16;
        }
        if (linhas[x] > COLUNA_A){ 
            peao_uma_casa[PRETAS][x] = x - 8;
        }
        if (linhas[x] > COLUNA_B){
            peao_duas_casas[PRETAS][x] = x - 16;
        }

    }
}

void init_cavalo_lookups(){
    int k, x = 0;
    
    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        k = 0;

        if (linhas[x] < 6 && colunas[x] < COLUNA_H){
            cavalo_moves[x][k++] = x + 17;
        }
        if (linhas[x] < COLUNA_H && colunas[x] < COLUNA_G){
            cavalo_moves[x][k++] = x + 10;
        }
        if (linhas[x] < 6 && colunas[x] > COLUNA_A){
            cavalo_moves[x][k++] = x + 15;
        }
        if (linhas[x] < COLUNA_H && colunas[x] > COLUNA_B){
            cavalo_moves[x][k++] = x + 6;
        }
        if (linhas[x] > 1 && colunas[x] < COLUNA_H){
            cavalo_moves[x][k++] = x - 15;
        }
        if (linhas[x] > COLUNA_A && colunas[x] < COLUNA_G){
            cavalo_moves[x][k++] = x - 6;
        }
        if (linhas[x] > 1 && colunas[x] > COLUNA_A){
            cavalo_moves[x][k++] = x - 17;
        }
        if (linhas[x] > COLUNA_A && colunas[x] > COLUNA_B){
            cavalo_moves[x][k++] = x - 10;
        }

        cavalo_moves[x][k] = -1;
    }

    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        k = 0;
        while (cavalo_moves[x][k] > -1){
            bit_moves_cavalo[x] |= mask[cavalo_moves[x][k]];
            k++;
        }
    }
}

void init_rei_lookups(){
    int x, y, k = 0;

    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        k = 0;
        y = 0;

        if (colunas[x] > COLUNA_A){
            rei_moves[x][y++] = x-1;    
        }
        if (colunas[x] < COLUNA_H){
            rei_moves[x][y++] = x+1;    
        }
        if (linhas[x] > COLUNA_A){
            rei_moves[x][y++] = x-8;    
        }
        if (linhas[x] < COLUNA_H){
            rei_moves[x][y++] = x+8;    
        }
        if (colunas[x] < COLUNA_H && linhas[x] < COLUNA_H){
            rei_moves[x][y++] = x+9;    
        }
        if (colunas[x] > COLUNA_A && linhas[x] < COLUNA_H){
            rei_moves[x][y++] = x+7;    
        }
        if (colunas[x] > COLUNA_A && linhas[x] > COLUNA_A){
            rei_moves[x][y++] = x-9;    
        }
        if (colunas[x] < COLUNA_H && linhas[x] > COLUNA_A){
            rei_moves[x][y++] = x-7;    
        }

        rei_moves[x][y] = -1;
    }

    k = 0;
    while (rei_moves[x][k] > -1){
        bit_moves_rei[x] |= mask[rei_moves[x][k]];
        k++;
    }
}

void init_dtb_lookups(){
    int x, y = 0;

    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        y=0;

        for (int z = 0; z < 8; z++){
            dtb_moves[x][z] = -1;
        }

        if (colunas[x] > COLUNA_A){
            dtb_moves[x][OESTE] = x-1;
            rei_moves[x][y++] = x-1;    
        }
        if (colunas[x] < COLUNA_H){
            dtb_moves[x][LESTE] = x+1;
            rei_moves[x][y++] = x+1;    
        }
        if (linhas[x] > COLUNA_A){
            dtb_moves[x][SUL] = x-8;
            rei_moves[x][y++] = x-8;    
        }
        if (linhas[x] < COLUNA_H){
            dtb_moves[x][NORTE] = x+8;
            rei_moves[x][y++] = x+8;    
        }
        if (colunas[x] < COLUNA_H && linhas[x] < COLUNA_H){
            dtb_moves[x][NE] = x+9;
            rei_moves[x][y++] = x+9;    
        }
        if (colunas[x] > COLUNA_A && linhas[x] < COLUNA_H){
            dtb_moves[x][NO] = x+7;
            rei_moves[x][y++] = x+7;    
        }
        if (colunas[x] > COLUNA_A && linhas[x] > COLUNA_A){
            dtb_moves[x][SO] = x-9;
            rei_moves[x][y++] = x-9;    
        }
        if (colunas[x] < COLUNA_H && linhas[x] > COLUNA_A){
            dtb_moves[x][SE] = x-7;
            rei_moves[x][y++] = x-7;    
        }

        rei_moves[x][y] = -1;
    }

    for (x = 0; x < CASAS_DO_TABULEIRO; x++){
        bit_moves_bispo[x] = 0;
        bit_moves_torre[x] = 0;
        bit_moves_dama[x] = 0;

        for (y = 0; y < CASAS_DO_TABULEIRO; y++){            
            if (x == y){
                continue;
            }

            if (no_diag[x] == no_diag[y] || ne_diag[x] == ne_diag[y]){
                bit_moves_bispo[x] |= mask[y];
                bit_moves_dama[x] |= mask[y];
            }

            if (colunas[x] == colunas[y] || linhas[x] == linhas[y]){
                bit_moves_torre[x] |= mask[y];
                bit_moves_dama[x] |= mask[y];
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

void adicionar_captura(const int x, const int casa, const int score){
    lista_de_lances[mc].inicio = x;
    lista_de_lances[mc].destino = casa;
    lista_de_lances[mc].score = score + SCORE_DE_CAPTURA;
    mc++;
}

void adicionar_lance(const int x, const int casa){
    lista_de_lances[mc].inicio = x;
    lista_de_lances[mc].destino = casa;
    lista_de_lances[mc].score = historico[x][casa];
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
            adicionar_lance(E1, G1);
        }
        if (roque & BRANCAS_ROQUE_MAIOR && !(bit_entre[A1][E1] & bit_total)){
            adicionar_lance(E1, C1);
        }
    }
    else{
        if (roque & PRETAS_ROQUE_MENOR && !(bit_entre[H8][E8] & bit_total)){
            adicionar_lance(E8, G8);
        }
        if (roque & PRETAS_ROQUE_MAIOR && !(bit_entre[A8][E8] & bit_total)){
            adicionar_lance(E8, C8);
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
                    casa_destino = dtb_moves[casa][direcao];

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
                    casa_destino = dtb_moves[casa][direcao];

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

        for (direcao = NORTE; direcao < NO + 1; direcao += 2){
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
                    casa_destino = dtb_moves[casa][direcao];

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
            t2 &= not_mask[casa];

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
            t2 &= not_mask[casa];

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
            t2 &= not_mask[casa];

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