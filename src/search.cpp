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

int Search::tempo_do_inicio, Search::tempo_do_fim;
int Search::lances_avaliados;

int Search::historico_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
Gen::lance Search::contraLance_heuristica[CASAS_DO_TABULEIRO][CASAS_DO_TABULEIRO];
Gen::lance Search::killers_primarios[MAX_PLY];
Gen::lance Search::killers_secundarios[MAX_PLY];

jmp_buf env;
bool parar_pesquisa;

int Interface::lance_inicio, Interface::lance_destino, Interface::lance_promove;

void verificar_tempo(){
    if ((Interface::obter_tempo() >= Search::tempo_do_fim || (Interface::tempo_maximo < 50 && Game::ply > 1)) && Interface::profundidade_fixa == 0 && Game::ply > 1){
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

    score[0] = Values::pieces_valor[Bitboard::tabuleiro[destino]];
    score[1] = Values::pieces_valor[Bitboard::tabuleiro[inicio]];

    int score_total = 0;

    while (recaptura < 10){
        if (!Update::fazer_captura(inicio, destino)){
            break;
        }

        recapturas_feitas++;
        recaptura++;

        // No node-counter increment here. `pesquisa_rapida` (the qsearch
        // entry) counts each qsearch invocation; the recapture loop is part
        // of a SEE-style evaluation, not independent search nodes. Counting
        // every recapture step inflated `lances_avaliados` and made the time
        // poll (`lances_avaliados & VERIFICACAO_DE_LANCES`) fire more often
        // than the search-node-count interpretation suggests.
        menor_recaptura = Attacks::menor_atacante(Game::lado, Game::xlado, destino); // ordena por MVA/LVV

        if (menor_recaptura > -1){
            score[recaptura + 1] = Values::pieces_valor[Bitboard::tabuleiro[menor_recaptura]];

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
        Update::desfaz_captura();
        recapturas_feitas--;
    }

    return score_total;
}

int pesquisa_rapida(int alpha, int beta){
    
    Search::lances_avaliados++;

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

        if (score_capturas + Values::pieces_valor[Bitboard::tabuleiro[Gen::lista_de_lances[i].destino]] < alpha){
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
                Hash::adicionar_hash(Game::lado, Gen::lista_de_lances[melhorlance],
                                     score_capturas, 0, TT_BOUND_LOWER,
                                     Gen::lista_de_lances[melhorlance].promove);
            }

            return score_capturas;
        }
    }

    return alpha;
}

void adicionar_pontuacao_iid(int alpha, int beta, int profundidade){
    for (int candidato = Game::qntt_lances_totais[Game::ply]; candidato < Game::qntt_lances_totais[Game::ply + 1]; ++candidato){
        Gen::lista_de_lances[candidato].score = -Search::pesquisa(-beta, -alpha, profundidade REDUCAO_IID, false);
    }
}

// Parameterized variant of Hash::adicionar_pontuacao_de_hash. The namespace
// version reads Hash::hash_{inicio,destino,promove} globals which any nested
// hash_lookup (inside recursive pesquisa) will stomp — so once we've recursed
// through captures, boosting by globals aims at a descendant's TT move, not
// ours. This version takes the move by value so stomping is impossible.
static void score_tt_move(const int tt_inicio, const int tt_destino, const int tt_promove){
    for (int lance = Game::qntt_lances_totais[Game::ply]; lance < Game::qntt_lances_totais[Game::ply + 1]; lance++){
        const Gen::lance &m = Gen::lista_de_lances[lance];
        if (m.inicio == tt_inicio && m.destino == tt_destino && m.promove == tt_promove){
            Gen::lista_de_lances[lance].score = PONTUACAO_HASH;
            return;
        }
    }
}

int Search::pesquisa(int alpha, int beta, int profundidade, bool pv, bool null_permitido){
    if (Game::ply && Game::checar_repeticoes()){
        return VALOR_EMPATE;
    }

    const int alpha_original = alpha;

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

    // TT probe: single lookup serves both cutoff attempt and move-ordering.
    const bool tt_hit   = Hash::hash_lookup(Game::lado);
    const int  tt_score = tt_hit ? Hash::hash_score : 0;
    const int  tt_depth = tt_hit ? Hash::hash_depth : -1;
    const int  tt_bound = tt_hit ? Hash::hash_bound : TT_BOUND_NONE;

    // Snapshot the TT move before any recursion can call hash_lookup and
    // stomp the Hash::hash_* globals. The second adicionar_pontuacao_de_hash
    // call (after lazy quiet generation) runs after we've recursed into
    // child searches for captures, each of which overwrites those globals
    // with their own TT entries — without this snapshot the second call
    // boosts an arbitrary move from a descendant position.
    const int tt_mv_inicio  = tt_hit ? Hash::hash_inicio  : 0;
    const int tt_mv_destino = tt_hit ? Hash::hash_destino : 0;
    const int tt_mv_promove = tt_hit ? Hash::hash_promove : 0;

    // TT cutoff: non-root, non-PV nodes only, stored depth must be >= current.
    if (tt_hit && !pv && Game::ply > 0 && tt_depth >= profundidade){
        if (tt_bound == TT_BOUND_EXACT)                      return tt_score;
        if (tt_bound == TT_BOUND_LOWER && tt_score >= beta)  return beta;
        if (tt_bound == TT_BOUND_UPPER && tt_score <= alpha) return alpha;
    }

    int check = 0;

    if (Attacks::casa_esta_sendo_atacada(Game::xlado, Bitboard::bitscan(Bitboard::bit_pieces[Game::lado][R]))){
        check = 1;
    }

    // Futility / reverse-futility pruning. Both depend on a static eval and
    // both are only safe at non-PV, non-check, low-depth nodes (where the
    // static eval is a reasonable proxy for the deep-search outcome).
    int static_eval = 0;
    const bool can_futility = !pv && !check && profundidade <= FUTILITY_DEPTH_THRESH;

    if (can_futility){
        static_eval = Eval::avaliar();

        // Reverse futility (a.k.a. static null-move): if our static eval is
        // already so far above beta that even an opponent's best play within
        // our margin can't pull us back, return early. The margin scales
        // with depth — deeper search has more room to swing.
        if (static_eval - FUTILITY_MARGIN_PER_PLY * profundidade >= beta){
            return static_eval;
        }
    }

    // Null-move pruning. If we hand the opponent a free move and they still
    // can't beat beta with a reduced-depth search, our actual move can only
    // produce an even better score — return beta. Guarded against:
    //   - in-check: passing would leave the king in check (illegal).
    //   - PV nodes: PVS expects exact scores; null-move's null-window result
    //     isn't usable.
    //   - already-null nodes: prevent two null-moves in a row, which would
    //     just be a deeper skip equivalent to a single bigger R.
    //   - low depth: at depth-1-R below 1 we'd hit qsearch, no info gained.
    //   - zugzwang risk: if we have only pawns + king, "passing" is often
    //     strictly worse than any move (opposition, etc.), so the null-move
    //     bet is unsafe.
    if (!check && !pv && null_permitido
        && profundidade >= 3
        && (Bitboard::bit_pieces[Game::lado][C] | Bitboard::bit_pieces[Game::lado][B]
          | Bitboard::bit_pieces[Game::lado][T] | Bitboard::bit_pieces[Game::lado][D])){
        const int reducao_null = (profundidade >= R_NULL_DEPTH_THRESH) ? R_NULL_HIGH : R_NULL_LOW;
        Update::fazer_null_move();
        const int score_null = -pesquisa(-beta, -beta + 1, profundidade - 1 - reducao_null, false, false);
        Update::desfaz_null_move();

        if (score_null >= beta){
            return beta;
        }
    }

    // Staged move generation.
    // - IID: needs the whole list up-front to score quiet moves, so full gen.
    // - TT hit whose destination holds an enemy piece: almost certainly a
    //   capture, so lazy quiet gen is safe (TT capture lives in captures
    //   list, gets boosted, tried first; if it cuts off we skip quiet gen).
    // - TT hit whose destination is empty: either a quiet move or EP. Full
    //   gen so the TT quiet is in the list and gets boosted to the top.
    //   (Phase A regressed here by searching all captures before discovering
    //   the TT quiet — this branch is what Phase B adds to prevent that.)
    // - No TT hit: pure lazy (captures now, quiets on exhaustion).
    const bool iid_path = !tt_hit && (profundidade > PROFUNDIDADE_CONDICAO_IID) && pv;

    bool quiets_generated;

    if (iid_path){
        Gen::gerar_lances(Game::lado, Game::xlado);
        adicionar_pontuacao_iid(alpha, beta, profundidade);
        quiets_generated = true;
    } else if (tt_hit){
        const bool tt_dest_has_enemy = Bitboard::mask[tt_mv_destino] & Bitboard::bit_lados[Game::xlado];
        if (tt_dest_has_enemy){
            Gen::gerar_capturas_busca(Game::lado, Game::xlado);
            score_tt_move(tt_mv_inicio, tt_mv_destino, tt_mv_promove);
            quiets_generated = false;
        } else {
            Gen::gerar_lances(Game::lado, Game::xlado);
            score_tt_move(tt_mv_inicio, tt_mv_destino, tt_mv_promove);
            quiets_generated = true;
        }
    } else {
        Gen::gerar_capturas_busca(Game::lado, Game::xlado);
        quiets_generated = false;
    }

    int lances_legais_na_posicao = 0;
    int score_candidato;
    int nova_profundidade;

    bool pesquisandoPV = true;

    for (int candidato = Game::qntt_lances_totais[Game::ply]; ; ++candidato){
        // Exhausted captures? Lazily generate quiets and re-boost the TT move
        // if it happens to be quiet (adicionar_pontuacao_de_hash is a no-op
        // when called a second time after already scoring a capture).
        if (candidato >= Game::qntt_lances_totais[Game::ply + 1]){
            if (quiets_generated) break;
            // Optimization: if forward futility would prune every quiet move
            // we'd generate (and we already have at least one legal move
            // searched, so we won't trigger a spurious mate/stalemate),
            // skip quiet generation entirely.
            if (can_futility
                && lances_legais_na_posicao > 0
                && static_eval + FUTILITY_MARGIN_PER_PLY * profundidade + FUTILITY_MARGIN_FP_EXTRA <= alpha){
                break;
            }
            // Reach here only when there was no TT hit, or when TT hit had
            // an enemy-occupied destination (capture path). In both cases
            // the TT move — if any — was already discovered and boosted
            // during the capture stage, so no re-boost is needed here.
            Gen::gerar_silenciosos(Game::lado, Game::xlado);
            quiets_generated = true;
            if (candidato >= Game::qntt_lances_totais[Game::ply + 1]) break;
        }

        ordenar_lances(candidato);

        // Forward futility pruning: at low depth with at least one legal
        // move already explored, skip quiet (non-capture, non-promotion)
        // moves whose optimistic value still can't beat alpha. Captures and
        // promotions are exempt because their material swing easily exceeds
        // the futility margin.
        if (can_futility
            && lances_legais_na_posicao > 0
            && Gen::lista_de_lances[candidato].promove == 0
            && !(Bitboard::mask[Gen::lista_de_lances[candidato].destino] & Bitboard::bit_total)
            && static_eval + FUTILITY_MARGIN_PER_PLY * profundidade + FUTILITY_MARGIN_FP_EXTRA <= alpha){
            continue;
        }

        // verifica se o lance é legal
        if (!Update::fazer_lance(Gen::lista_de_lances[candidato].inicio, Gen::lista_de_lances[candidato].destino, Gen::lista_de_lances[candidato].promove)){
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
                Update::desfaz_lance();
                continue;
            }
        }

        Update::desfaz_lance();

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

                Hash::adicionar_hash(Game::lado, Gen::lista_de_lances[candidato],
                                     score_candidato, profundidade, TT_BOUND_LOWER,
                                     Gen::lista_de_lances[candidato].promove);

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

    const int tt_store_bound = (melhorscore > alpha_original) ? TT_BOUND_EXACT : TT_BOUND_UPPER;
    Hash::adicionar_hash(Game::lado, melhorlance, melhorscore, profundidade, tt_store_bound, melhorlance.promove);

    return alpha;
}

void Search::pensar(bool verbose){
    int melhor_linha;
    int alpha, beta;

    parar_pesquisa = false;

    setjmp(env);
    if (parar_pesquisa){
        while (Game::ply){
            // The unwinder must dispatch to desfaz_null_move for any
            // null-move frame on the stack. Naively calling desfaz_lance
            // on the (0, 0, captura=VAZIO) sentinel would corrupt the
            // bitboards and Zobrist key (mover_piece(0, 0) clears tabuleiro[0]).
            Update::desfaz_lance_ou_null();
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

    if (verbose && Interface::protocolo_ativo != Interface::PROTO_UCI){
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
                if (Interface::protocolo_ativo == Interface::PROTO_UCI){
                    int elapsed_ms = Interface::obter_tempo() - tempo_do_inicio;
                    if (elapsed_ms < 1){
                        elapsed_ms = 1;
                    }
                    int nps_uci = (int)((double)lances_avaliados * 1000.0 / (double)elapsed_ms);

                    printf("info depth %d seldepth %d time %d nodes %d nps %d ",
                        profundidade, profundidade, elapsed_ms, lances_avaliados, nps_uci);

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
                    Interface::obter_pv_uci(pv_buf, sizeof(pv_buf), profundidade);
                    printf("pv %s\n", pv_buf);
                    fflush(stdout);
                }
                else{
                    printf("%d %d %d %d ", profundidade, melhor_linha, (Interface::obter_tempo() - tempo_do_inicio) / 10, lances_avaliados);
                    Interface::exibir_melhor_linha(profundidade);
                    printf("\n");
                    fflush(stdout);
                }
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

