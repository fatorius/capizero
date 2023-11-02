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

int historico_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
lance contraLance_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];

jmp_buf env;
bool parar_pesquisa;

int max_atual;

int lance_inicio, lance_destino;

void verificar_tempo(){
    if ((obter_tempo() >= tempo_do_fim || (tempo_maximo < 50 && ply > 1)) && profundidade_fixa == 0 && ply > 1){
        parar_pesquisa = true;
        longjmp(env, 0);
    }
}

void ordenar_lances(const int desde){
    lance l;

    int maior_score = lista_de_lances[desde].score;
    int indice_do_maior_score = desde;

    for (int i = desde + 1; i < qntt_lances_totais[ply + 1]; ++i){
        if (lista_de_lances[i].score > maior_score){
            maior_score = lista_de_lances[i].score;
            indice_do_maior_score = i;
        }

        l = lista_de_lances[desde];
        lista_de_lances[desde] = lista_de_lances[indice_do_maior_score];
        lista_de_lances[indice_do_maior_score] = l; 
    }
}

int pesquisa_quiescence(int inicio, const int destino){
    int menor_recaptura;
    int recaptura = 0;
    int recapturas_feitas = 0;

    int score[12];

    memset(score, 0, sizeof(score));

    score[0] = pieces_valor[tabuleiro[destino]];
    score[1] = pieces_valor[tabuleiro[inicio]];

    int score_total = 0;

    while (recaptura < 10){
        if (!fazer_captura(inicio, destino)){
            break;
        }

        recapturas_feitas++;
        lances_avaliados++;
        recaptura++;

        menor_recaptura = menor_atacante(lado, xlado, destino); // ordena por MVA/LVV

        if (menor_recaptura > -1){
            score[recaptura + 1] = pieces_valor[tabuleiro[menor_recaptura]];

            if (score[recaptura] > (score[recaptura - 1] + score[recaptura + 1])){
                recaptura--;
                break;
            }
        }
        else{
            break;
        }

        inicio = menor_recaptura;
    }

    while (recaptura > 1){
        if (score[recaptura-1] >= score[recaptura-2]){
            recaptura -= 2;
        }
        else {
            break;
        }
    }

    for (int piece = 0; piece < recaptura; piece++){
        if (piece % 2 == 0){
            score_total += score[piece];
        }
        else{
            score_total -= score[piece];
        }
    }

    while (recapturas_feitas){
        desfaz_captura();
        recapturas_feitas--;
    }

    return score_total;
}

int pesquisa_rapida(int alpha, int beta){
    
    lances_avaliados++;

    int score_capturas = avaliar();

    if (score_capturas > alpha){
        if (score_capturas >= beta){
            return beta;
        }

        alpha = score_capturas;
    }
    else if (score_capturas + VALOR_DAMA < alpha){
        return alpha;
    }

    int score = 0; 
    int melhorlance = 0;
    int melhorscore = 0;

    gerar_capturas(lado, xlado);

    for (int i = qntt_lances_totais[ply]; i < qntt_lances_totais[ply+1]; ++i){
        ordenar_lances(i);

        if (score_capturas + pieces_valor[tabuleiro[lista_de_lances[i].destino]] < alpha){
            continue;
        }

        score = pesquisa_quiescence(lista_de_lances[i].inicio, lista_de_lances[i].destino);

        if (score > melhorscore){
            melhorscore = score;
            melhorlance = i;
        }
    }

    if (melhorscore > 0){
        score_capturas += melhorscore;
    }

    if (score_capturas > alpha){
        if (score_capturas >= beta){
            if (melhorscore > 0){
                adicionar_hash(lado, lista_de_lances[melhorlance]);
            }

            return score_capturas;
        }
    }

    return alpha;
}

int pesquisa(int alpha, int beta, int profundidade){
    if (ply && checar_repeticoes()){
        return VALOR_EMPATE;
    }

    if (profundidade < 1){
        return pesquisa_rapida(alpha, beta);
    }

    lances_avaliados++;

    if ((lances_avaliados & VERIFICACAO_DE_LANCES) == 0){
        verificar_tempo();
    }

    if (ply > MAX_PLY-2){
        return avaliar();
    }


    // pesquisa alpha-beta

    lance melhorlance;

    int melhorscore = MELHOR_SCORE_INICIAL;

    int check = 0;

    if (casa_esta_sendo_atacada(xlado, bitscan(bit_pieces[lado][R]))){
        check = 1;
    }

    gerar_lances(lado, xlado);

    if (hash_lookup(lado)){
        adicionar_pontuacao_de_hash(); // ordena por lances hash, pv é pesquisado primeiro
    } // else {internal interactive deeping} TODO

    int lances_legais_na_posicao = 0;
    int score_candidato;
    int nova_profundidade;

    bool pesquisandoPV = true;

    for (int candidato = qntt_lances_totais[ply]; candidato < qntt_lances_totais[ply + 1]; ++candidato){
        ordenar_lances(candidato);

        // verifica se o lance é legal
        if (!fazer_lance(lista_de_lances[candidato].inicio, lista_de_lances[candidato].destino)){
            continue;
        }

        lances_legais_na_posicao++;


        // REDUÇÕES E EXTENSÕES
        if (casa_esta_sendo_atacada(xlado, bitscan(bit_pieces[lado][R]))){ // extensões
            nova_profundidade = profundidade; // extensões de xeques
        }
        else{ // reduções
            nova_profundidade = profundidade - 3;
        
            // TODO MELHORAR ESSAS REDUÇÕES ?????????????
            if (lista_de_lances[candidato].score > SCORE_DE_CAPTURA_VANTAJOSAS || lances_legais_na_posicao == 1 || check == 1){
                nova_profundidade = profundidade - 1;
            }
            else if (lista_de_lances[candidato].score > 0){
                nova_profundidade = profundidade - 2;
            }
        }

        // pesquisa da variante principal (pvs)
        if (pesquisandoPV){
            score_candidato = -pesquisa(-beta, -alpha, nova_profundidade); // extender profundidade???     
        }
        else{
            if (-pesquisa(-alpha - 1, -alpha, nova_profundidade) > alpha){
                score_candidato = -pesquisa(-beta, -alpha, nova_profundidade); 
            }
            else{
                desfaz_lance();
                continue;
            }
        }

        desfaz_lance();

        if (score_candidato > melhorscore){
            melhorscore = score_candidato;
            melhorlance = lista_de_lances[candidato];
        }

        if (score_candidato > alpha){
            if (score_candidato >= beta){ // beta-cutoff

                if (!(mask[lista_de_lances[candidato].destino] & bit_total)){ // adiciona no historico se não for uma captura
                    historico_heuristica[lista_de_lances[candidato].inicio][lista_de_lances[candidato].destino] += 1 << profundidade; 
                    contraLance_heuristica[lista_do_jogo[hply].inicio][lista_do_jogo[hply].destino] = lista_de_lances[candidato];
                }

                adicionar_hash(lado, lista_de_lances[candidato]);

                // TODO adicionar killer move

                return beta;
            }
            alpha = score_candidato;
            pesquisandoPV = false;
        }
    }

    if (!lances_legais_na_posicao){
        if (casa_esta_sendo_atacada(xlado, bitscan(bit_pieces[lado][R]))){
            return VALOR_XEQUE_MATE_PADRAO - ply;
        }
        else{
            return VALOR_EMPATE;
        }
    }

    if (cinquenta >= 100){
        return VALOR_EMPATE;
    }

    adicionar_hash(lado, melhorlance);

    return alpha;
}

void pensar(bool verbose){
    int melhor_linha;

    parar_pesquisa = false;

    setjmp(env);
    if (parar_pesquisa){
        while (ply){
            desfaz_lance();
        }

        return;
    }

    if (!tempo_fixo){
        if (casa_esta_sendo_atacada(xlado, bitscan(bit_pieces[lado][R]))){
            tempo_maximo = tempo_maximo / 2;
        }
    }

    tempo_do_inicio = obter_tempo();
    tempo_do_fim = tempo_do_inicio + tempo_maximo;

    ply = 0;
    lances_avaliados = 0;

    nova_posicao();

    memset(historico_heuristica, 0, sizeof(historico_heuristica));
    memset(contraLance_heuristica, 0, sizeof(contraLance_heuristica));

    if (verbose){
        printf("ply score time nodes pv\n");
    }

    for (int profundidade = 1; profundidade <= profundidade_maxima; ++profundidade){
        max_atual = profundidade;

        if (!profundidade_fixa && profundidade_maxima > 1){
            if (tempo_fixo){
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

        if (hash_lookup(lado)){
            if (verbose){
                printf("%d %d %d %d ", profundidade, melhor_linha, (obter_tempo() - tempo_do_inicio) / 10, lances_avaliados);
                exibir_melhor_linha(profundidade);    
                printf("\n");
                fflush(stdout);    
            }  
        }
        else{
            lance_inicio = 0;
            lance_destino = 0;
        }


        if (melhor_linha >= VALOR_XEQUE_MATE_BRANCAS || melhor_linha <= VALOR_XEQUE_MATE_PRETAS){
            break;
        }
    }
}

