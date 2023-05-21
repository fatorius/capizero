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

int lances_avaliados;
int tempo_do_inicio, tempo_do_fim;
int historico[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];

jmp_buf env;
bool parar_pesquisa;

int max_atual;

int lance_inicio, lance_destino;

void exibir_melhor_linha(int profundidade){

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

