#include "search.h"

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include "update.h"
#include "game.h"
#include "interface.h"
#include "attacks.h"
#include "bitboard.h"
#include "eval.h"
#include "hash.h"
#include "gen.h"

int lances_avaliados;
int tempo_do_inicio, tempo_do_fim;
int historico[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];

jmp_buf env;
bool parar_pesquisa;

int max_atual;

int lance_inicio, lance_destino;

void exibir_melhor_linha(int profundidade){
     lance_inicio = hash_inicio;
     lance_destino = hash_destino;

     for (int x = 0; x < profundidade; x++){
        if (hash_lookup(lado) == false){
            break;
        }

        printf(" ");
        print_lance_algebrico(hash_inicio, hash_destino);
        fazer_lance(hash_inicio, hash_destino);
     }

     while (ply){
        desfaz_lance();
     }
     
}

void nova_posicao(){
    int c = 0;

    piece_mat[BRANCAS] = peao_mat[BRANCAS] = 0;
    piece_mat[PRETAS] = peao_mat[PRETAS] = 0;

    for (int i = 0; i < CASAS_DO_TABULEIRO; i++){
        if (tabuleiro[i] < VAZIO){
            if (bit_lados[BRANCAS] & mask[i]){
                c = BRANCAS;
            }
            else{
                c = PRETAS;
            }

            adicionar_piece(c, tabuleiro[i], i);
        }
    }

    chaveAtual = obter_chave();
    lockAtual = obter_lock();
}

int checar_repeticoes(){
    for (int i = hply-4; i >= hply - cinquenta; i -=2){
        if (lista_do_jogo[i].hash == chaveAtual && lista_do_jogo[i].lock == lockAtual){
            return 1;
        }
    }
    return 0;
}

void verificar_tempo(){
    if ((obter_tempo() >= tempo_do_fim || (tempo_maximo < 50 && ply > 1)) && profundidade_fixa == 0 && ply > 1){
        parar_pesquisa = true;
        longjmp(env, 0);
    }
}

void set_lance_hash(){
    for (int x = primeiro_lance[ply]; x < primeiro_lance[ply + 1]; x++){
        if (lista_de_lances[x].inicio == hash_inicio && lista_de_lances[x].destino == hash_destino){
            lista_de_lances[x].score = PONTUACAO_HASH;
            return;
        }
    }
}

void ordenar_lances(const int desde){
    lance l;

    int ls = lista_de_lances[desde].score;
    int li = desde;

    for (int i = desde + 1; i < primeiro_lance[ply + 1]; ++i){
        if (lista_de_lances[i].score > ls){
            ls = lista_de_lances[i].score;
            li = i;
        }

        l = lista_de_lances[desde];
        lista_de_lances[desde] = lista_de_lances[li];
        lista_de_lances[li] = l; 
    }
}

int pesquisa_de_recapturas(int inicio, const int destino){

    int b;
    int c = 0;
    int t = 0;

    int score[12];

    memset(score, 0, sizeof(score));

    score[0] = pieces_valor[tabuleiro[destino]];
    score[1] = pieces_valor[tabuleiro[inicio]];

    int score_total = 0;

    while (c < 10){
        if (!fazer_captura(inicio, destino)){
            break;
        }

        t++;
        lances_avaliados++;
        c++;

        b = menor_atacante(lado, xlado, destino);

        if (b > -1){
            score[c + 1] = pieces_valor[tabuleiro[b]];

            if (score[c] > (score[c - 1] + score[c + 1])){
                c--;
                break;
            }
        }
        else{
            break;
        }

        inicio = b;
    }

    while (c > 1){
        if (score[c-1] >= score[c-2]){
            c -= 2;
        }
        else {
            break;
        }
    }

    for (int x = 0; x < c; x++){
        if (x % 2 == 0){
            score_total += score[x];
        }
        else{
            score_total -= score[x];
        }
    }

    while (t){
        desfaz_captura();
        t--;
    }

    return score_total;
}

int pesquisa_de_capturas(int alpha, int beta){
    
    lances_avaliados++;

    int x = avaliar();

    if (x > alpha){
        if (x >= beta){
            return beta;
        }

        alpha = x;
    }
    else if (x + VALOR_DAMA < alpha){
        return alpha;
    }

    int score = 0; 
    int melhorlance = 0;
    int melhorscore = 0;

    gerar_capturas(lado, xlado);

    for (int i = primeiro_lance[ply]; i < primeiro_lance[ply+1]; i++){
        ordenar_lances(i);

        if (x + pieces_valor[tabuleiro[lista_de_lances[i].destino]] < alpha){
            continue;
        }

        score = pesquisa_de_recapturas(lista_de_lances[i].inicio, lista_de_lances[i].destino);

        if (score > melhorscore){
            melhorscore = score;
            melhorlance = i;
        }
    }

    if (melhorscore > 0){
        x += melhorscore;
    }

    if (x > alpha){
        if (x >= beta){
            if (melhorscore > 0){
                adicionar_hash(lado, lista_de_lances[melhorlance]);
            }

            return x;
        }
    }

    return alpha;
}

int pesquisa(int alpha, int beta, int profundidade){
    if (ply && checar_repeticoes()){
        return 0;
    }

    if (profundidade < 1){
        return pesquisa_de_capturas(alpha, beta);
    }

    lances_avaliados++;

    if ((lances_avaliados & 4095) == 0){
        verificar_tempo();
    }

    if (ply > MAX_PLY-2){
        return avaliar();
    }

    lance melhorlance;

    int melhorscore = MELHOR_SCORE_INICIAL;

    int check = 0;

    if (casa_esta_sendo_atacada(xlado, bitscan(bit_pieces[lado][R]))){
        check = 1;
    }

    gerar_lances(lado, xlado);

    if (hash_lookup(lado)){
        set_lance_hash();
    }

    int c = 0;
    int x;
    int p;

    for (int i = primeiro_lance[ply]; i < primeiro_lance[ply + 1]; ++i){
        ordenar_lances(i);

        // verifica se o lance é legal
        if (!fazer_lance(lista_de_lances[i].inicio, lista_de_lances[i].destino)){
            continue;
        }

        c++;

        if (casa_esta_sendo_atacada(xlado, bitscan(bit_pieces[lado][R]))){
            p = profundidade;
        }
        else{
            p = profundidade - 3;
        
            if (lista_de_lances[i].score > SCORE_DE_CAPTURA || c == 1 || check == 1){
                p = profundidade - 1;
            }
            else if (lista_de_lances[i].score > 0){
                p = profundidade - 2;
            }
        }

        x = -pesquisa(-beta, -alpha, p);

        desfaz_lance();

        if (x > melhorscore){
            melhorscore = x;
            melhorlance = lista_de_lances[i];
        }

        if (x > alpha){
            if (x >= beta){
                if (!(mask[lista_de_lances[i].destino] & bit_total)){
                    historico[lista_de_lances[i].inicio][lista_de_lances[i].destino] += profundidade;
                }
                adicionar_hash(lado, lista_de_lances[i]);
                return beta;
            }
            alpha = x;
        }
    }

    if (c == 0){
        if (casa_esta_sendo_atacada(xlado, bitscan(bit_pieces[lado][R]))){
            return VALOR_XEQUE_MATE_PADRAO + ply;
        }
        else{
            return 0;
        }
    }

    if (cinquenta >= 100){
        return 0;
    }

    adicionar_hash(lado, melhorlance);

    return alpha;
}

void pensar(){
    int melhor_linha;

    parar_pesquisa = false;

    setjmp(env);
    if (parar_pesquisa){
        while (ply){
            desfaz_lance();
        }

        return;
    }

    if (tempo_fixo == 0){
        if (casa_esta_sendo_atacada(xlado, bitscan(bit_pieces[lado][R]))){
            tempo_maximo = tempo_maximo / 2;
        }
    }

    tempo_do_inicio = obter_tempo();
    tempo_do_fim = tempo_do_inicio + tempo_maximo;

    ply = 0;
    lances_avaliados = 0;

    nova_posicao();

    memset(historico, 0, sizeof(historico));

    printf("ply score time nodes pv\n");

    for (int profundidade = 1; profundidade <= profundidade_maxima; ++profundidade){
        max_atual = profundidade;

        if (profundidade_fixa == 0 && profundidade_maxima > 1){
            if (tempo_fixo == 1){
                if (obter_tempo() >= tempo_do_inicio + tempo_maximo){
                    parar_pesquisa = true;
                    return;
                }
            }
            else if (obter_tempo() >= tempo_do_inicio + tempo_maximo/4){
                parar_pesquisa = true;
                return;
            }
        }

        melhor_linha = pesquisa(ALPHA_INICIAL, BETA_INICIAL, profundidade);

        printf("%d %d %d %d ", profundidade, melhor_linha, (obter_tempo() - tempo_do_inicio) / 10, lances_avaliados);

        if (hash_lookup(lado)){
            exibir_melhor_linha(profundidade);
        }
        else{
            lance_inicio = 0;
            lance_destino = 0;
        }

        printf("\n");
        fflush(stdout);

        if (melhor_linha >= VALOR_XEQUE_MATE_BRANCAS || melhor_linha <= VALOR_XEQUE_MATE_PRETAS){
            break;
        }
    }
}

