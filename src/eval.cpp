#include "eval.h"

#include <cstring>

#include "bitboard.h"
#include "consts.h"
#include "game.h"
#include "gen.h"
#include "params.h"
#include "estado.h"


Eval::Score Eval::score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];

const int Eval::phase_weights[6] = {
    PHASE_PEAO, PHASE_CAVALO, PHASE_BISPO, PHASE_TORRE, PHASE_DAMA, PHASE_REI
};

Eval::Score Eval::mobilidade_cavalo[9];
Eval::Score Eval::mobilidade_bispo[14];
Eval::Score Eval::mobilidade_torre[15];
Eval::Score Eval::mobilidade_dama[28];

Eval::Score Eval::ks_weight_c, Eval::ks_weight_b, Eval::ks_weight_t, Eval::ks_weight_d;

Eval::Score Eval::passado[8];
Eval::Score Eval::peao_isolado;
Eval::Score Eval::coluna_semi_aberta, Eval::coluna_aberta;
Eval::Score Eval::par_de_bispos;
Eval::Score Eval::escudo[6];

static Bitboard::u64 king_zone[CASAS_DO_TABULEIRO];

bool Eval::usar_hash_peoes = true;

void Eval::init_eval_tables(){
    for (int x = 0; x < CASAS_DO_TABULEIRO; x++){
        const int xf = Consts::flip[x];

        score_casas[BRANCAS][P][x] = make_score(Values::peao_score_mg[x], Values::peao_score_eg[x]);
        score_casas[BRANCAS][C][x] = make_score(Values::cavalo_score_mg[x], Values::cavalo_score_eg[x]);
        score_casas[BRANCAS][B][x] = make_score(Values::bispo_score_mg[x], Values::bispo_score_eg[x]);
        score_casas[BRANCAS][T][x] = make_score(Values::torre_score_mg[x], Values::torre_score_eg[x]);
        score_casas[BRANCAS][D][x] = make_score(Values::dama_score_mg[x], Values::dama_score_eg[x]);
        score_casas[BRANCAS][R][x] = make_score(Values::rei_score_mg[x], Values::rei_score_eg[x]);

        score_casas[PRETAS][P][x] = make_score(Values::peao_score_mg[xf], Values::peao_score_eg[xf]);
        score_casas[PRETAS][C][x] = make_score(Values::cavalo_score_mg[xf], Values::cavalo_score_eg[xf]);
        score_casas[PRETAS][B][x] = make_score(Values::bispo_score_mg[xf], Values::bispo_score_eg[xf]);
        score_casas[PRETAS][T][x] = make_score(Values::torre_score_mg[xf], Values::torre_score_eg[xf]);
        score_casas[PRETAS][D][x] = make_score(Values::dama_score_mg[xf], Values::dama_score_eg[xf]);
        score_casas[PRETAS][R][x] = make_score(Values::rei_score_mg[xf], Values::rei_score_eg[xf]);
    }

    for (int i = 0; i < 8; i++){
        passado[i] = make_score(Values::peao_passado_mg[i], Values::peao_passado_eg[i]);
    }
    for (int i = 0; i < 6; i++){
        escudo[i] = make_score(Values::escudo_mg[i], Values::escudo_eg[i]);
    }
    peao_isolado       = make_score(PEAO_ISOLADO_MG, PEAO_ISOLADO_EG);
    coluna_semi_aberta = make_score(COLUNA_SEMI_ABERTA_MG, COLUNA_SEMI_ABERTA_EG);
    coluna_aberta      = make_score(COLUNA_ABERTA_MG, COLUNA_ABERTA_EG);
    par_de_bispos      = make_score(BISHOP_PAIR_MG, BISHOP_PAIR_EG);

    for (int i = 0; i < 9; i++){
        mobilidade_cavalo[i] = make_score(Values::mobilidade_cavalo_mg[i], Values::mobilidade_cavalo_eg[i]);
    }
    for (int i = 0; i < 14; i++){
        mobilidade_bispo[i] = make_score(Values::mobilidade_bispo_mg[i], Values::mobilidade_bispo_eg[i]);
    }
    for (int i = 0; i < 15; i++){
        mobilidade_torre[i] = make_score(Values::mobilidade_torre_mg[i], Values::mobilidade_torre_eg[i]);
    }
    for (int i = 0; i < 28; i++){
        mobilidade_dama[i] = make_score(Values::mobilidade_dama_mg[i], Values::mobilidade_dama_eg[i]);
    }

    ks_weight_c = make_score(KS_WEIGHT_C, 0);
    ks_weight_b = make_score(KS_WEIGHT_B, 0);
    ks_weight_t = make_score(KS_WEIGHT_T, 0);
    ks_weight_d = make_score(KS_WEIGHT_D, 0);
}

// Depends on Gen::bit_moves_rei, so it must run after Gen::init_lookup_tables().
void Eval::init_king_zone(){
    for (int x = 0; x < CASAS_DO_TABULEIRO; x++){
        king_zone[x] = Gen::bit_moves_rei[x] | Bitboard::mask[x];
    }
}

uint64_t Eval::zona_do_rei(int casa){
    return king_zone[casa];
}

int Eval::fase(const Estado &est){
    return (est.fase_valor > PHASE_MAX) ? PHASE_MAX : est.fase_valor;
}

void Eval::atualizar_materiais(Estado &est){
    int cor;

    est.peao_mat[BRANCAS] = 0;
    est.peao_mat[PRETAS] = 0;

    est.piece_mat[BRANCAS] = 0;
    est.piece_mat[PRETAS] = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        if (est.tabuleiro[casa] < VAZIO){
            if (est.bit_lados[BRANCAS] & Bitboard::mask[casa]){
                cor = BRANCAS;
            }
            else{
                cor = PRETAS;
            }

            if (est.tabuleiro[casa] == P){
                est.peao_mat[cor] += VALOR_PEAO;
            }
            else{
                est.piece_mat[cor] += Values::pieces_valor[est.tabuleiro[casa]];
            }
        }
    }
}

int Eval::indice_escudo(int l, int casa){
    const int fileira = Bitboard::fileiras[l][casa];
    if (fileira != 1 && fileira != 2){
        return -1;
    }
    const int coluna = Consts::colunas[casa];
    if (coluna >= COLUNA_F){
        return (fileira - 1) * 3 + (coluna - COLUNA_F);
    }
    if (coluna <= COLUNA_C){
        return (fileira - 1) * 3 + (COLUNA_C - coluna);
    }
    return -1;
}

static Eval::Score avaliar_peao(const Estado &est, const int l, const int casa,
                                Eval::Score &ala_do_rei, Eval::Score &ala_da_dama){
    Eval::Score score = 0;

    if (!(Bitboard::mask_passados[l][casa] & est.bit_pieces[l^1][P]) && !(Bitboard::mask_path[l][casa] & est.bit_pieces[l][P])){
        score += Eval::passado[Bitboard::fileiras[l][casa]];
    }

    if ((Bitboard::mask_isolados[casa] & est.bit_pieces[l][P]) == 0){
        score += Eval::peao_isolado;
    }

    const int e = Eval::indice_escudo(l, casa);
    ala_do_rei = (e >= 0 && Consts::colunas[casa] >= COLUNA_F) ? Eval::escudo[e] : 0;
    ala_da_dama = (e >= 0 && Consts::colunas[casa] <= COLUNA_C) ? Eval::escudo[e] : 0;

    return score;
}

void Eval::limpar_hash_peoes(Estado &est){
    memset(est.hash_peoes, 0, sizeof(est.hash_peoes));
}

static Eval::EntradaPeoes& estrutura_de_peoes(Estado &est){
    const Bitboard::u64 pb = est.bit_pieces[BRANCAS][P];
    const Bitboard::u64 pp = est.bit_pieces[PRETAS][P];

    const Bitboard::u64 chave = (pb * 0x9E3779B97F4A7C15ULL + pp) * 0xC2B2AE3D27D4EB4FULL;
    Eval::EntradaPeoes &e = Eval::usar_hash_peoes ? est.hash_peoes[chave >> (64 - PAWN_HASH_BITS)] : est.entrada_avulsa;

    // Entrada zerada equivale a "sem peões": score 0 e escudo 0, que é o valor correto.
    if (Eval::usar_hash_peoes && e.peoes[BRANCAS] == pb && e.peoes[PRETAS] == pp){
        return e;
    }

    e.peoes[BRANCAS] = pb;
    e.peoes[PRETAS] = pp;

    for (int l = 0; l < LADOS; l++){
        Eval::Score score = 0;
        Eval::Score ala_do_rei = 0;
        Eval::Score ala_da_dama = 0;

        Bitboard::u64 t1 = est.bit_pieces[l][P];
        while (t1){
            const int casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score += Eval::score_casas[l][P][casa];
            score += avaliar_peao(est, l, casa, ala_do_rei, ala_da_dama);
        }

        e.score[l] = score;
        e.ala_do_rei[l] = ala_do_rei;
        e.ala_da_dama[l] = ala_da_dama;
    }

    return e;
}

static Eval::Score avaliar_torre(const Estado &est, const int l, const int casa){
    if (!(Bitboard::mask_cols[casa] & est.bit_pieces[l][P])){
        if (!(Bitboard::mask_cols[casa] & est.bit_pieces[l^1][P])){
            return Eval::coluna_aberta;
        }
        return Eval::coluna_semi_aberta;
    }
    return 0;
}

int Eval::avaliar(Estado &est){
    Score score[LADOS] = {0, 0};

    const Eval::EntradaPeoes &peoes = estrutura_de_peoes(est);

    Bitboard::u64 t1;
    int casa;

    const int rei_branco_sq = Bitboard::bitscan(est.bit_pieces[BRANCAS][R]);
    const int rei_preto_sq  = Bitboard::bitscan(est.bit_pieces[PRETAS][R]);
    const int rei_sq[LADOS] = { rei_branco_sq, rei_preto_sq };
    const Bitboard::u64 zona_inimiga[LADOS] = {
        king_zone[rei_preto_sq],
        king_zone[rei_branco_sq]
    };

    const int kw_c = mg_score(ks_weight_c);
    const int kw_b = mg_score(ks_weight_b);
    const int kw_t = mg_score(ks_weight_t);
    const int kw_d = mg_score(ks_weight_d);

    for (int l = 0; l < LADOS; l++){
        const Bitboard::u64 nao_proprios = ~est.bit_lados[l];
        const Bitboard::u64 zona = zona_inimiga[l];
        int pressao_rei = 0;

        score[l] += score_casas[l][R][rei_sq[l]];

        score[l] += peoes.score[l];

        t1 = est.bit_pieces[l][C];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][C][casa];
            const Bitboard::u64 att = Gen::bit_moves_cavalo[casa];
            score[l] += mobilidade_cavalo[Bitboard::popcount(att & nao_proprios)];
            pressao_rei += Bitboard::popcount(att & zona) * kw_c;
        }

        t1 = est.bit_pieces[l][B];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][B][casa];
            const Bitboard::u64 att = Gen::atacantes_bispo(est, casa);
            score[l] += mobilidade_bispo[Bitboard::popcount(att & nao_proprios)];
            pressao_rei += Bitboard::popcount(att & zona) * kw_b;
        }

        if (Bitboard::popcount(est.bit_pieces[l][B]) >= 2){
            score[l] += par_de_bispos;
        }

        t1 = est.bit_pieces[l][T];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][T][casa];
            score[l] += avaliar_torre(est, l, casa);
            const Bitboard::u64 att = Gen::atacantes_torre(est, casa);
            score[l] += mobilidade_torre[Bitboard::popcount(att & nao_proprios)];
            pressao_rei += Bitboard::popcount(att & zona) * kw_t;
        }

        t1 = est.bit_pieces[l][D];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][D][casa];
            const Bitboard::u64 att = Gen::atacantes_bispo(est, casa) | Gen::atacantes_torre(est, casa);
            score[l] += mobilidade_dama[Bitboard::popcount(att & nao_proprios)];
            pressao_rei += Bitboard::popcount(att & zona) * kw_d;
        }

        score[l] += make_score((pressao_rei * pressao_rei) / KS_SCALE, 0);
    }

    for (int l = 0; l < LADOS; l++){
        if (est.bit_pieces[l][R] & Bitboard::mask_ala_do_rei){
            score[l] += peoes.ala_do_rei[l];
        }
        else if (est.bit_pieces[l][R] & Bitboard::mask_ala_da_dama){
            score[l] += peoes.ala_da_dama[l];
        }
    }

    const Score diff = score[est.lado] - score[est.xlado];
    const int mg_diff = mg_score(diff);
    const int eg_diff = eg_score(diff);
    const int phase = fase(est);
    return (mg_diff * phase + eg_diff * (PHASE_MAX - phase)) / PHASE_MAX;
}