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
Gen::lance contraLance_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
Gen::lance killers_primarios[MAX_PLY];
Gen::lance killers_secundarios[MAX_PLY];

jmp_buf env;
bool parar_pesquisa;

int Interface::lance_inicio, Interface::lance_destino;

void verificar_tempo(){
    if ((Interface::obter_tempo() >= tempo_do_fim || (Interface::tempo_maximo < 50 && Game::ply > 1)) && Interface::profundidade_fixa == 0 && Game::ply > 1){
        parar_pesquisa = true;
        longjmp(env, 0);
    }
}

void ordenar_lances(const int desde){
    Gen::lance l;

    int maior_score = Gen::lista_de_lances[desde].score;
    int indice_do_maior_score = desde;

    for (int i = desde + 1; i < Game::qntt_lances_totais[Game::ply + 1]; ++i){
        if (Gen::lista_de_lances[i].score > maior_score){
            maior_score = Gen::lista_de_lances[i].score;
            indice_do_maior_score = i;
        }
    }

    l = Gen::lista_de_lances[desde];
    Gen::lista_de_lances[desde] = Gen::lista_de_lances[indice_do_maior_score];
    Gen::lista_de_lances[indice_do_maior_score] = l; 
}

int pesquisa_quiescence(int inicio, const int destino){
    int menor_recaptura;
    int recaptura = 0;
    int recapturas_feitas = 0;

    int score[12];

    memset(score, 0, sizeof(score));

    score[0] = pieces_valor[Bitboard::tabuleiro[destino]];
    score[1] = pieces_valor[Bitboard::tabuleiro[inicio]];

    int score_total = 0;

    while (recaptura < 10){
        if (!fazer_captura(inicio, destino)){
            break;
        }

        recapturas_feitas++;
        lances_avaliados++;
        recaptura++;

        menor_recaptura = Attacks::menor_atacante(Game::lado, Game::xlado, destino); // ordena por MVA/LVV

        if (menor_recaptura > -1){
            score[recaptura + 1] = pieces_valor[Bitboard::tabuleiro[menor_recaptura]];

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

    int score_capturas = Eval::avaliar();

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

    Gen::gerar_capturas(Game::lado, Game::xlado);

    for (int i = Game::qntt_lances_totais[Game::ply]; i < Game::qntt_lances_totais[Game::ply+1]; ++i){
        ordenar_lances(i);

        if (score_capturas + pieces_valor[Bitboard::tabuleiro[Gen::lista_de_lances[i].destino]] < alpha){
            continue;
        }

        score = pesquisa_quiescence(Gen::lista_de_lances[i].inicio, Gen::lista_de_lances[i].destino);

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
                Hash::adicionar_hash(Game::lado, Gen::lista_de_lances[melhorlance]);
            }

            return score_capturas;
        }
    }

    return alpha;
}

void adicionar_pontuacao_iid(int alpha, int beta, int profundidade){
    for (int candidato = Game::qntt_lances_totais[Game::ply]; candidato < Game::qntt_lances_totais[Game::ply + 1]; ++candidato){
        Gen::lista_de_lances[candidato].score = -pesquisa(-beta, -alpha, profundidade REDUCAO_IID, false);
    }
}   

int pesquisa(int alpha, int beta, int profundidade, bool pv){
    if (Game::ply && Game::checar_repeticoes()){
        return VALOR_EMPATE;
    }

    if (profundidade < 1){
        return pesquisa_rapida(alpha, beta);
    }

    lances_avaliados++;

    if ((lances_avaliados & VERIFICACAO_DE_LANCES) == 0){
        verificar_tempo();
    }

    if (Game::ply > MAX_PLY-2){
        return Eval::avaliar();
    }


    // pesquisa alpha-beta

    Gen::lance melhorlance;

    int melhorscore = MELHOR_SCORE_INICIAL;

    int check = 0;

    if (Attacks::casa_esta_sendo_atacada(Game::xlado, Bitboard::bitscan(Bitboard::bit_pieces[Game::lado][R]))){
        check = 1;
    }

    Gen::gerar_lances(Game::lado, Game::xlado);

    if (Hash::hash_lookup(Game::lado)){
        Hash::adicionar_pontuacao_de_hash(); // ordena por lances hash, pv é pesquisado primeiro
    } else if (profundidade > PROFUNDIDADE_CONDICAO_IID && pv){
        adicionar_pontuacao_iid(alpha, beta, profundidade);
    }

    int lances_legais_na_posicao = 0;
    int score_candidato;
    int nova_profundidade;

    bool pesquisandoPV = true;

    for (int candidato = Game::qntt_lances_totais[Game::ply]; candidato < Game::qntt_lances_totais[Game::ply + 1]; ++candidato){
        ordenar_lances(candidato);

        // verifica se o lance é legal
        if (!fazer_lance(Gen::lista_de_lances[candidato].inicio, Gen::lista_de_lances[candidato].destino)){
            continue;
        }

        lances_legais_na_posicao++;


        // REDUÇÕES E EXTENSÕES
        if (check == 1){ // extensões
            nova_profundidade = profundidade; // extensões de xeques
        }
        else{ // reduções
            if (Gen::lista_de_lances[candidato].score > SCORE_DE_CAPTURA_VANTAJOSAS || lances_legais_na_posicao == 1){
                nova_profundidade = profundidade - 1;
            }
            else if (Gen::lista_de_lances[candidato].score > 0){
                nova_profundidade = profundidade - 2;
            }
            else{
                nova_profundidade = profundidade - REDUCAO_LMR;
            }
        }

        // pesquisa da variante principal (pvs)
        if (pesquisandoPV){
            score_candidato = -pesquisa(-beta, -alpha, nova_profundidade, true); // extender profundidade???     
        }
        else{
            if (-pesquisa(-alpha - 1, -alpha, nova_profundidade, false) > alpha){
                score_candidato = -pesquisa(-beta, -alpha, nova_profundidade, true); 
            }
            else{
                desfaz_lance();
                continue;
            }
        }

        desfaz_lance();

        if (score_candidato > melhorscore){
            melhorscore = score_candidato;
            melhorlance = Gen::lista_de_lances[candidato];
        }

        if (score_candidato > alpha){
            if (score_candidato >= beta){ // beta-cutoff

                if (!(Bitboard::mask[Gen::lista_de_lances[candidato].destino] & Bitboard::bit_total)){ // adiciona no historico se não for uma captura
                    historico_heuristica[Gen::lista_de_lances[candidato].inicio][Gen::lista_de_lances[candidato].destino] += 1 << profundidade; 
                    contraLance_heuristica[Game::lista_do_jogo[Game::hply].inicio][Game::lista_do_jogo[Game::hply].destino] = Gen::lista_de_lances[candidato];

                    killers_secundarios[Game::ply] = killers_primarios[Game::ply];
                    killers_primarios[Game::ply] = Gen::lista_de_lances[candidato];
                }

                Hash::adicionar_hash(Game::lado, Gen::lista_de_lances[candidato]);

                return beta;
            }
            alpha = score_candidato;
            pesquisandoPV = false;
        }
    }

    if (!lances_legais_na_posicao){
        if (Attacks::casa_esta_sendo_atacada(Game::xlado, Bitboard::bitscan(Bitboard::bit_pieces[Game::lado][R]))){
            return VALOR_XEQUE_MATE_PADRAO - Game::ply;
        }
        else{
            return VALOR_EMPATE;
        }
    }

    if (Game::cinquenta >= 100){
        return VALOR_EMPATE;
    }

    Hash::adicionar_hash(Game::lado, melhorlance);

    return alpha;
}

void pensar(bool verbose){
    int melhor_linha;
    int alpha, beta;

    parar_pesquisa = false;

    setjmp(env);
    if (parar_pesquisa){
        while (Game::ply){
            desfaz_lance();
        }

        return;
    }

    if (!Interface::tempo_fixo){
        if (Attacks::casa_esta_sendo_atacada(Game::xlado, Bitboard::bitscan(Bitboard::bit_pieces[Game::lado][R]))){
            Interface::tempo_maximo = Interface::tempo_maximo / 2;
        }
    }

    tempo_do_inicio = Interface::obter_tempo();
    tempo_do_fim = tempo_do_inicio + Interface::tempo_maximo;

    Game::ply = 0;
    lances_avaliados = 0;

    Game::nova_posicao();

    memset(historico_heuristica, 0, sizeof(historico_heuristica));
    memset(contraLance_heuristica, 0, sizeof(contraLance_heuristica));

    if (verbose){
        printf("ply score time nodes pv\n");
    }

    for (int profundidade = 1; profundidade <= Interface::profundidade_maxima; ++profundidade){
        if (!Interface::profundidade_fixa && Interface::profundidade_maxima > 1){
            if (Interface::tempo_fixo){
                if (Interface::obter_tempo() >= tempo_do_inicio + Interface::tempo_maximo){
                    parar_pesquisa = true;
                    return;
                }
            }
            else if (Interface::obter_tempo() >= tempo_do_inicio + Interface::tempo_maximo/4){
                parar_pesquisa = true;
                return;
            }
        }

        if (profundidade == 1){
            alpha = ALPHA_INICIAL;
            beta = BETA_INICIAL;
        }
        else{
            alpha = melhor_linha - TAMANHO_JANELA_DE_PESQUISA;
            beta = melhor_linha + TAMANHO_JANELA_DE_PESQUISA;
        }

        melhor_linha = pesquisa(alpha, beta, profundidade, true);

        if (melhor_linha <= alpha){
            alpha = (melhor_linha - (TAMANHO_JANELA_DE_PESQUISA * READAPTACAO_JANELA_DE_PESQUISA));
            melhor_linha = pesquisa(alpha, beta, profundidade, true);
        }
        else if (melhor_linha >= beta){
            if (profundidade == 1){
                beta = BETA_INICIAL;
            }
            else{
                beta = (melhor_linha + (TAMANHO_JANELA_DE_PESQUISA * READAPTACAO_JANELA_DE_PESQUISA));
            }
            melhor_linha = pesquisa(alpha, beta, profundidade, true);
        }

        if (Hash::hash_lookup(Game::lado)){
            if (verbose){
                printf("%d %d %d %d ", profundidade, melhor_linha, (Interface::obter_tempo() - tempo_do_inicio) / 10, lances_avaliados);
                Interface::exibir_melhor_linha(profundidade);    
                printf("\n");
                fflush(stdout);    
            }  
        }
        else{
            Interface::lance_inicio = 0;
            Interface::lance_destino = 0;
        }


        if (melhor_linha >= VALOR_XEQUE_MATE_BRANCAS || melhor_linha <= VALOR_XEQUE_MATE_PRETAS){
            break;
        }
    }
}

