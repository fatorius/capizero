#include "search.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <atomic>

#include "update.h"
#include "game.h"
#include "interface.h"
#include "attacks.h"
#include "bitboard.h"
#include "eval.h"
#include "hash.h"
#include "gen.h"
#include "estado.h"

int Search::tempo_do_inicio, Search::tempo_do_fim;

// Redução LMR pré-computada: lmr_tabela[profundidade][lances_legais]
static int lmr_tabela[LMR_TABLE_DEPTH][LMR_TABLE_MOVES];

void Search::init_lmr(){
    for (int d = 0; d < LMR_TABLE_DEPTH; d++){
        for (int m = 0; m < LMR_TABLE_MOVES; m++){
            int r = 1;
            if (d > 0 && m > 0){
                r = (int)(log((double)d) * log((double)m) / LMR_DIVISOR);
                if (r < 1) r = 1;
            }
            lmr_tabela[d][m] = r;
        }
    }
}

// Pedido de parada da busca. Quando ligado, cada nó de pesquisa() desfaz o
// próprio lance e retorna sem gravar na TT nem mexer em history/killers,
// e pensar() descarta a iteração incompleta.
std::atomic<bool> parar_pesquisa(false);

static inline bool pesquisa_parada(){
    return parar_pesquisa.load(std::memory_order_relaxed);
}

int Interface::lance_inicio, Interface::lance_destino, Interface::lance_promove;

void verificar_tempo(const Estado &est){
    if ((Interface::obter_tempo() >= Search::tempo_do_fim || (Interface::tempo_maximo < 50 && est.ply > 1)) && Interface::profundidade_fixa == 0 && est.ply > 1){
        parar_pesquisa.store(true, std::memory_order_relaxed);
    }
}

void ordenar_lances(Estado &est, const int desde){
    Gen::lance l;

    int maior_score = est.lista_de_lances[desde].score;
    int indice_do_maior_score = desde;

    for (int i = desde + 1; i < est.qntt_lances_totais[est.ply + 1]; ++i){
        if (est.lista_de_lances[i].score > maior_score){
            maior_score = est.lista_de_lances[i].score;
            indice_do_maior_score = i;
        }
    }

    l = est.lista_de_lances[desde];
    est.lista_de_lances[desde] = est.lista_de_lances[indice_do_maior_score];
    est.lista_de_lances[indice_do_maior_score] = l; 
}

int pesquisa_quiescence(Estado &est, int inicio, const int destino){
    int menor_recaptura;
    int recaptura = 0;
    int recapturas_feitas = 0;

    int score[12];

    memset(score, 0, sizeof(score));

    score[0] = Values::pieces_valor[est.tabuleiro[destino]];
    score[1] = Values::pieces_valor[est.tabuleiro[inicio]];

    int score_total = 0;

    while (recaptura < 10){
        if (!Update::fazer_captura(est, inicio, destino)){
            break;
        }

        recapturas_feitas++;
        recaptura++;

        menor_recaptura = Attacks::menor_atacante(est, est.lado, est.xlado, destino); // ordena por MVA/LVV

        if (menor_recaptura > -1){
            score[recaptura + 1] = Values::pieces_valor[est.tabuleiro[menor_recaptura]];

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
        Update::desfaz_captura(est);
        recapturas_feitas--;
    }

    return score_total;
}

int pesquisa_rapida(Estado &est, int alpha, int beta){
    
    est.lances_avaliados++;

    int score_capturas = Eval::avaliar(est);

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

    Gen::gerar_capturas(est, est.lado, est.xlado);

    for (int i = est.qntt_lances_totais[est.ply]; i < est.qntt_lances_totais[est.ply+1]; ++i){
        ordenar_lances(est, i);

        if (score_capturas + Values::pieces_valor[est.tabuleiro[est.lista_de_lances[i].destino]] < alpha){
            continue;
        }

        score = pesquisa_quiescence(est, est.lista_de_lances[i].inicio, est.lista_de_lances[i].destino);

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
                Hash::adicionar_hash(est, est.lado, est.lista_de_lances[melhorlance],
                                     score_capturas, 0, TT_BOUND_LOWER,
                                     est.lista_de_lances[melhorlance].promove);
            }

            return score_capturas;
        }
    }

    return alpha;
}

void adicionar_pontuacao_iid(Estado &est, int alpha, int beta, int profundidade){
    for (int candidato = est.qntt_lances_totais[est.ply]; candidato < est.qntt_lances_totais[est.ply + 1]; ++candidato){
        est.lista_de_lances[candidato].score = -Search::pesquisa(est, -beta, -alpha, profundidade REDUCAO_IID, false);
        if (pesquisa_parada()){
            return;
        }
    }
}

// Parameterized variant of Hash::adicionar_pontuacao_de_hash. The namespace
// version reads Hash::hash_{inicio,destino,promove} globals which any nested
// hash_lookup (inside recursive pesquisa) will stomp — so once we've recursed
// through captures, boosting by globals aims at a descendant's TT move, not
// ours. This version takes the move by value so stomping is impossible.
static void score_tt_move(Estado &est, const int tt_inicio, const int tt_destino, const int tt_promove){
    for (int lance = est.qntt_lances_totais[est.ply]; lance < est.qntt_lances_totais[est.ply + 1]; lance++){
        const Gen::lance &m = est.lista_de_lances[lance];
        if (m.inicio == tt_inicio && m.destino == tt_destino && m.promove == tt_promove){
            est.lista_de_lances[lance].score = PONTUACAO_HASH;
            return;
        }
    }
}

int Search::pesquisa(Estado &est, int alpha, int beta, int profundidade, bool pv, bool null_permitido){
    if (est.ply && Game::checar_repeticoes(est)){
        return VALOR_EMPATE;
    }

    const int alpha_original = alpha;

    if (profundidade < 1){
        return pesquisa_rapida(est, alpha, beta);
    }

    est.lances_avaliados++;

    if ((est.lances_avaliados & VERIFICACAO_DE_LANCES) == 0){
        verificar_tempo(est);
    }

    if (pesquisa_parada()){
        return 0;
    }

    if (est.ply > MAX_PLY-2){
        return Eval::avaliar(est);
    }


    // pesquisa alpha-beta

    Gen::lance melhorlance;

    int melhorscore = MELHOR_SCORE_INICIAL;

    // TT probe: single lookup serves both cutoff attempt and move-ordering.
    const bool tt_hit   = Hash::hash_lookup(est, est.lado);
    const int  tt_score = tt_hit ? est.hash_score : 0;
    const int  tt_depth = tt_hit ? est.hash_depth : -1;
    const int  tt_bound = tt_hit ? est.hash_bound : TT_BOUND_NONE;

    const int tt_mv_inicio  = tt_hit ? est.hash_inicio  : 0;
    const int tt_mv_destino = tt_hit ? est.hash_destino : 0;
    const int tt_mv_promove = tt_hit ? est.hash_promove : 0;

    // TT cutoff: non-root, non-PV nodes only, stored depth must be >= current.
    if (tt_hit && !pv && est.ply > 0 && tt_depth >= profundidade){
        if (tt_bound == TT_BOUND_EXACT)                      return tt_score;
        if (tt_bound == TT_BOUND_LOWER && tt_score >= beta)  return beta;
        if (tt_bound == TT_BOUND_UPPER && tt_score <= alpha) return alpha;
    }

    int check = 0;

    if (Attacks::casa_esta_sendo_atacada(est, est.xlado, Bitboard::bitscan(est.bit_pieces[est.lado][R]))){
        check = 1;
    }

    // Futility / reverse-futility pruning. 
    int static_eval = 0;
    const bool side_has_pieces =
        est.bit_pieces[est.lado][C] | est.bit_pieces[est.lado][B]
      | est.bit_pieces[est.lado][T] | est.bit_pieces[est.lado][D];
    const bool can_futility = !pv && !check && profundidade <= FUTILITY_DEPTH_THRESH;

    if (can_futility){
        static_eval = Eval::avaliar(est);

        // Reverse futility (a.k.a. static null-move). 
        if (side_has_pieces
            && beta < VALOR_XEQUE_MATE_BRANCAS - MAX_PLY
            && static_eval - FUTILITY_MARGIN_PER_PLY * profundidade >= beta){
            return (static_eval + beta) / 2;
        }
    }

    // Null-move pruning.
    if (!check && !pv && null_permitido
        && profundidade >= 3
        && (est.bit_pieces[est.lado][C] | est.bit_pieces[est.lado][B]
          | est.bit_pieces[est.lado][T] | est.bit_pieces[est.lado][D])){
        const int reducao_null = (profundidade >= R_NULL_DEPTH_THRESH) ? R_NULL_HIGH : R_NULL_LOW;
        Update::fazer_null_move(est);
        const int score_null = -pesquisa(est, -beta, -beta + 1, profundidade - 1 - reducao_null, false, false);
        Update::desfaz_null_move(est);

        if (pesquisa_parada()){
            return 0;
        }

        if (score_null >= beta){
            return beta;
        }
    }

    // Staged move generation.
    const bool iid_path = !tt_hit && (profundidade > PROFUNDIDADE_CONDICAO_IID) && pv;

    bool quiets_generated;

    if (iid_path){
        Gen::gerar_lances(est, est.lado, est.xlado);
        adicionar_pontuacao_iid(est, alpha, beta, profundidade);
        if (pesquisa_parada()){
            return 0;
        }
        quiets_generated = true;
    } else if (tt_hit){
        const bool tt_dest_has_enemy = Bitboard::mask[tt_mv_destino] & est.bit_lados[est.xlado];
        if (tt_dest_has_enemy){
            Gen::gerar_capturas_busca(est, est.lado, est.xlado);
            score_tt_move(est, tt_mv_inicio, tt_mv_destino, tt_mv_promove);
            quiets_generated = false;
        } else {
            Gen::gerar_lances(est, est.lado, est.xlado);
            score_tt_move(est, tt_mv_inicio, tt_mv_destino, tt_mv_promove);
            quiets_generated = true;
        }
    } else {
        Gen::gerar_capturas_busca(est, est.lado, est.xlado);
        quiets_generated = false;
    }

    int lances_legais_na_posicao = 0;
    int score_candidato;
    int nova_profundidade;

    bool pesquisandoPV = true;

    for (int candidato = est.qntt_lances_totais[est.ply]; ; ++candidato){
        // Exhausted captures?
        if (candidato >= est.qntt_lances_totais[est.ply + 1]){
            if (quiets_generated) break;
            // Optimization: if forward futility would prune every quiet move
            // we'd generate (and we already have at least one legal move
            // searched, so we won't trigger a spurious mate/stalemate),
            // skip quiet generation entirely. Mate-distance guard mirrors
            // the per-move FP check below — we never skip moves when we
            // might be escaping (or delivering) a forced mate.
            if (can_futility
                && lances_legais_na_posicao > 0
                && alpha > VALOR_XEQUE_MATE_PRETAS + MAX_PLY
                && static_eval + FUTILITY_MARGIN_PER_PLY * profundidade + FUTILITY_MARGIN_FP_EXTRA <= alpha){
                break;
            }
            // Reach here only when there was no TT hit, or when TT hit had
            // an enemy-occupied destination (capture path). 
            Gen::gerar_silenciosos(est, est.lado, est.xlado);
            quiets_generated = true;
            if (candidato >= est.qntt_lances_totais[est.ply + 1]) break;
        }

        ordenar_lances(est, candidato);

        // Forward futility pruning
        if (can_futility
            && lances_legais_na_posicao > 0
            && alpha > VALOR_XEQUE_MATE_PRETAS + MAX_PLY
            && est.lista_de_lances[candidato].promove == 0
            && !(Bitboard::mask[est.lista_de_lances[candidato].destino] & est.bit_total)
            && static_eval + FUTILITY_MARGIN_PER_PLY * profundidade + FUTILITY_MARGIN_FP_EXTRA <= alpha){
            continue;
        }

        // verifica se o lance é legal
        if (!Update::fazer_lance(est, est.lista_de_lances[candidato].inicio, est.lista_de_lances[candidato].destino, est.lista_de_lances[candidato].promove)){
            continue;
        }

        lances_legais_na_posicao++;


        // EXTENSÕES E REDUÇÕES (LMR)
        if (check == 1){
            nova_profundidade = profundidade; // extensão de xeque
        }
        else if (est.lista_de_lances[candidato].score > SCORE_DE_CAPTURA_VANTAJOSAS
                 || lances_legais_na_posicao == 1){
            nova_profundidade = profundidade - 1; // primeira peça ou captura vantajosa: sem redução
        }
        else if (lances_legais_na_posicao >= 2 && profundidade >= 3){
            // LMR: redução logarítmica a partir do 2º lance legal, em nós PV e não-PV.
            // Não reduz: lance da TT e promoções (capturas vantajosas e 1º lance já saíram no ramo anterior).
            // Killers, contralances, capturas perdedoras e lances de histórico SÃO reduzidos.
            const bool nao_reduzir =
                est.lista_de_lances[candidato].score >= PONTUACAO_HASH
                || est.lista_de_lances[candidato].promove != 0;

            if (nao_reduzir){
                nova_profundidade = profundidade - 1;
            }
            else{
                const int lmr_d = profundidade < LMR_TABLE_DEPTH ? profundidade : LMR_TABLE_DEPTH - 1;
                const int lmr_m = lances_legais_na_posicao < LMR_TABLE_MOVES ? lances_legais_na_posicao : LMR_TABLE_MOVES - 1;
                const int lmr_r = lmr_tabela[lmr_d][lmr_m];
                nova_profundidade = profundidade - 1 - lmr_r;
                if (nova_profundidade < 1) nova_profundidade = 1;
            }
        }
        else{
            nova_profundidade = profundidade - 1;
        }

        // EXTENSÃO DE PEÃO: +1 ply para peão que chega à 7ª fileira (relativa).
        // Em xeque o nó já mantém a profundidade, então não empilha.
        int ext = 0;
        if (check == 0
            && est.lista_de_lances[candidato].promove == 0
            && est.tabuleiro[est.lista_de_lances[candidato].destino] == P
            // Após fazer_lance, o lado que moveu é Game::xlado.
            && Bitboard::fileiras[est.xlado][est.lista_de_lances[candidato].destino] == EXT_PEAO_FILEIRA){
            ext = 1;
        }
        nova_profundidade += ext;

        // pesquisa da variante principal (pvs)
        if (pesquisandoPV){
            score_candidato = -pesquisa(est, -beta, -alpha, nova_profundidade, true);
        }
        else{
            if (-pesquisa(est, -alpha - 1, -alpha, nova_profundidade, false) > alpha && !pesquisa_parada()){
                score_candidato = -pesquisa(est, -beta, -alpha, profundidade - 1 + ext, true);
            }
            else{
                Update::desfaz_lance(est);
                if (pesquisa_parada()){
                    return 0;
                }
                continue;
            }
        }

        Update::desfaz_lance(est);

        if (pesquisa_parada()){
            return 0;
        }

        if (score_candidato > melhorscore){
            melhorscore = score_candidato;
            melhorlance = est.lista_de_lances[candidato];
        }

        if (score_candidato > alpha){
            if (score_candidato >= beta){ // beta-cutoff

                if (!(Bitboard::mask[est.lista_de_lances[candidato].destino] & est.bit_total)){ // adiciona no historico se não for uma captura
                    est.historico_heuristica[est.lista_de_lances[candidato].inicio][est.lista_de_lances[candidato].destino] += 1 << profundidade; 
                    est.contraLance_heuristica[est.lista_do_jogo[est.hply].inicio][est.lista_do_jogo[est.hply].destino] = est.lista_de_lances[candidato];

                    est.killers_secundarios[est.ply] = est.killers_primarios[est.ply];
                    est.killers_primarios[est.ply] = est.lista_de_lances[candidato];
                }

                Hash::adicionar_hash(est, est.lado, est.lista_de_lances[candidato],
                                     score_candidato, profundidade, TT_BOUND_LOWER,
                                     est.lista_de_lances[candidato].promove);

                return beta;
            }
            alpha = score_candidato;
            pesquisandoPV = false;
        }
    }

    if (!lances_legais_na_posicao){
        if (Attacks::casa_esta_sendo_atacada(est, est.xlado, Bitboard::bitscan(est.bit_pieces[est.lado][R]))){
            return VALOR_XEQUE_MATE_PADRAO + est.ply;
        }
        else{
            return VALOR_EMPATE;
        }
    }

    if (est.cinquenta >= 100){
        return VALOR_EMPATE;
    }

    const int tt_store_bound = (melhorscore > alpha_original) ? TT_BOUND_EXACT : TT_BOUND_UPPER;
    Hash::adicionar_hash(est, est.lado, melhorlance, melhorscore, profundidade, tt_store_bound, melhorlance.promove);

    return alpha;
}

void Search::pensar(Estado &est, bool verbose){
    int melhor_linha;
    int alpha, beta;

    parar_pesquisa.store(false, std::memory_order_relaxed);

    if (!Interface::tempo_fixo){
        if (Attacks::casa_esta_sendo_atacada(est, est.xlado, Bitboard::bitscan(est.bit_pieces[est.lado][R]))){
            Interface::tempo_maximo = Interface::tempo_maximo / 2;
        }
    }

    tempo_do_inicio = Interface::obter_tempo();
    tempo_do_fim = tempo_do_inicio + Interface::tempo_maximo;

    est.ply = 0;
    est.lances_avaliados = 0;

    Game::nova_posicao(est);

    memset(est.historico_heuristica, 0, sizeof(est.historico_heuristica));
    memset(est.contraLance_heuristica, 0, sizeof(est.contraLance_heuristica));

    for (int profundidade = 1; profundidade <= Interface::profundidade_maxima; ++profundidade){
        if (!Interface::profundidade_fixa && Interface::profundidade_maxima > 1){
            if (Interface::tempo_fixo){
                if (Interface::obter_tempo() >= tempo_do_inicio + Interface::tempo_maximo){
                    parar_pesquisa.store(true, std::memory_order_relaxed);
                    return;
                }
            }
            else if (Interface::obter_tempo() >= tempo_do_inicio + Interface::tempo_maximo/4){
                parar_pesquisa.store(true, std::memory_order_relaxed);
                return;
            }
        }

        int delta = TAMANHO_JANELA_DE_PESQUISA;
        if (profundidade == 1){
            alpha = ALPHA_INICIAL;
            beta  = BETA_INICIAL;
        }
        else{
            alpha = melhor_linha - delta;
            beta  = melhor_linha + delta;
        }

        int fails = 0;
        while (true){
            melhor_linha = pesquisa(est, alpha, beta, profundidade, true);

            if (pesquisa_parada()){
                // Cada frame já desfez o próprio lance; a iteração incompleta
                // é descartada e o bestmove continua sendo o da anterior.
#ifdef DEBUG_BUILD
                assert(est.ply == 0);
                assert(est.chaveAtual == Hash::obter_chave(est) && est.lockAtual == Hash::obter_lock(est));
#endif
                return;
            }

            if (melhor_linha > alpha && melhor_linha < beta) break;
            if (++fails > MAX_ASPIRATION_FAILS) break;

            if (fails == MAX_ASPIRATION_FAILS){
                // Final attempt — full window, guaranteed to return a usable
                // score on the next iteration.
                alpha = ALPHA_INICIAL;
                beta  = BETA_INICIAL;
                continue;
            }

            delta *= 2;
            if (melhor_linha <= alpha){
                alpha = melhor_linha - delta;
                if (alpha < ALPHA_INICIAL) alpha = ALPHA_INICIAL;
            }
            else{
                beta = melhor_linha + delta;
                if (beta > BETA_INICIAL) beta = BETA_INICIAL;
            }
        }

        if (Hash::hash_lookup(est, est.lado)){
            if (verbose){
                int elapsed_ms = Interface::obter_tempo() - tempo_do_inicio;
                if (elapsed_ms < 1){
                    elapsed_ms = 1;
                }
                int nps_uci = (int)((double)est.lances_avaliados * 1000.0 / (double)elapsed_ms);

                printf("info depth %d seldepth %d time %d nodes %d nps %d ",
                    profundidade, profundidade, elapsed_ms, est.lances_avaliados, nps_uci);

                if (melhor_linha >= VALOR_XEQUE_MATE_BRANCAS - MAX_PLY){
                    int mate_in = (VALOR_XEQUE_MATE_BRANCAS - melhor_linha + 1) / 2;
                    printf("score mate %d ", mate_in);
                }
                else if (melhor_linha <= VALOR_XEQUE_MATE_PRETAS + MAX_PLY){
                    int mate_in = (melhor_linha - VALOR_XEQUE_MATE_PRETAS) / 2;
                    printf("score mate %d ", mate_in);
                }
                else{
                    printf("score cp %d ", melhor_linha);
                }

                static char pv_buf[512];
                Interface::obter_pv_uci(est, pv_buf, sizeof(pv_buf), profundidade);
                printf("pv %s\n", pv_buf);
                fflush(stdout);
            }
        }
        else{
            Interface::lance_inicio  = 0;
            Interface::lance_destino = 0;
            Interface::lance_promove = 0;
        }


        if (melhor_linha >= VALOR_XEQUE_MATE_BRANCAS || melhor_linha <= VALOR_XEQUE_MATE_PRETAS){
            break;
        }
    }
}

