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
    if (obter_tempo() >= tempo_do_fim || (tempo_maximo < 50 && ply > 1) && profundidade_fixa == 0 && ply > 1){
        parar_pesquisa = true;
        longjmp(env, 0);
    }
}

void set_lance_hash(){
    for (int x = primeiro_lance[ply]; x < primeiro_lance[ply + 1]; x++){
        if (lista_de_lances[x].inicio == hash_inicio && lista_de_lances[x].destino == hash_destino){
            lista_de_lances[x].score == PONTUACAO_HASH;
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

int pesquisa(int alpha, int beta, int profundidade){

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

    printf("ply      nodes  score  pv\n");

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

