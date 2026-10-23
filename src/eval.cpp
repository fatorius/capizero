#include "eval.h"

#include <cstring>

#include "bitboard.h"
#include "consts.h"
#include "game.h"
#include "gen.h"
#include "params.h"


Eval::Score Eval::score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
int Eval::passados[LADOS][CASAS_DO_TABULEIRO];

int Eval::peao_mat[LADOS];
int Eval::piece_mat[LADOS];

int Eval::fase_valor = 0;

const int Eval::phase_weights[6] = {
    PHASE_PEAO, PHASE_CAVALO, PHASE_BISPO, PHASE_TORRE, PHASE_DAMA, PHASE_REI
};

Eval::Score Eval::mobilidade_cavalo[9];
Eval::Score Eval::mobilidade_bispo[14];
Eval::Score Eval::mobilidade_torre[15];
Eval::Score Eval::mobilidade_dama[28];

Eval::Score Eval::ks_weight_c, Eval::ks_weight_b, Eval::ks_weight_t, Eval::ks_weight_d;

static Bitboard::u64 king_zone[CASAS_DO_TABULEIRO];

// Tudo que depende só da posição dos peões (PST, passados, isolados e
// escudo do rei) vive na hash de peões. A chave é o par de bitboards de
// peões, comparado por inteiro: não há colisão possível.
struct EntradaPeoes{
    Bitboard::u64 peoes[LADOS];
    Eval::Score score[LADOS];
    int16_t ala_do_rei[LADOS];
    int16_t ala_da_dama[LADOS];
};

static EntradaPeoes hash_peoes[PAWN_HASH_SIZE];
static EntradaPeoes entrada_avulsa;

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

        passados[BRANCAS][x] = Values::peao_passado_score[Consts::flip[x]];
        passados[PRETAS][x] = Values::peao_passado_score[x];
    }

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

    limpar_hash_peoes();
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

int Eval::fase(){
    return (fase_valor > PHASE_MAX) ? PHASE_MAX : fase_valor;
}

void Eval::atualizar_materiais(){
    int cor;

    peao_mat[BRANCAS] = 0;
    peao_mat[PRETAS] = 0;

    piece_mat[BRANCAS] = 0;
    piece_mat[PRETAS] = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        if (Bitboard::tabuleiro[casa] < VAZIO){
            if (Bitboard::bit_lados[BRANCAS] & Bitboard::mask[casa]){
                cor = BRANCAS;
            }
            else{
                cor = PRETAS;
            }

            if (Bitboard::tabuleiro[casa] == P){
                peao_mat[cor] += VALOR_PEAO;
            }
            else{
                piece_mat[cor] += Values::pieces_valor[Bitboard::tabuleiro[casa]];
            }
        }
    }
}

static int avaliar_peao(const int l, const int casa, int &ala_do_rei, int &ala_da_dama){
    int score = 0;

    if (!(Bitboard::mask_passados[l][casa] & Bitboard::bit_pieces[l^1][P]) && !(Bitboard::mask_path[l][casa] & Bitboard::bit_pieces[l][P])){
        score += Eval::passados[l][casa];
    }

    if ((Bitboard::mask_isolados[casa] & Bitboard::bit_pieces[l][P]) == 0){
        score -= ISOLADO_SCORE;
    }

    ala_do_rei = Values::defesa_ala_do_rei[l][casa];
    ala_da_dama = Values::defesa_ala_da_dama[l][casa];

    return score;
}

void Eval::limpar_hash_peoes(){
    memset(hash_peoes, 0, sizeof(hash_peoes));
}

static EntradaPeoes& estrutura_de_peoes(){
    const Bitboard::u64 pb = Bitboard::bit_pieces[BRANCAS][P];
    const Bitboard::u64 pp = Bitboard::bit_pieces[PRETAS][P];

    const Bitboard::u64 chave = (pb * 0x9E3779B97F4A7C15ULL + pp) * 0xC2B2AE3D27D4EB4FULL;
    EntradaPeoes &e = Eval::usar_hash_peoes ? hash_peoes[chave >> (64 - PAWN_HASH_BITS)] : entrada_avulsa;

    // Entrada zerada equivale a "sem peões": score 0 e escudo 0, que é o valor correto.
    if (Eval::usar_hash_peoes && e.peoes[BRANCAS] == pb && e.peoes[PRETAS] == pp){
        return e;
    }

    e.peoes[BRANCAS] = pb;
    e.peoes[PRETAS] = pp;

    for (int l = 0; l < LADOS; l++){
        Eval::Score score = 0;
        int ala_do_rei = 0;
        int ala_da_dama = 0;

        Bitboard::u64 t1 = Bitboard::bit_pieces[l][P];
        while (t1){
            const int casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score += Eval::score_casas[l][P][casa];
            const int peao_b = avaliar_peao(l, casa, ala_do_rei, ala_da_dama);
            score += Eval::make_score(peao_b, peao_b);
        }

        e.score[l] = score;
        e.ala_do_rei[l] = (int16_t)ala_do_rei;
        e.ala_da_dama[l] = (int16_t)ala_da_dama;
    }

    return e;
}

int avaliar_torre(const int l, const int casa){
    int score = 0;

    if (!(Bitboard::mask_cols[casa] & Bitboard::bit_pieces[l][P])){
        score = COLUNA_SEMI_ABERTA_BONUS;

        if (!(Bitboard::mask_cols[casa] & Bitboard::bit_pieces[l^1][P])){
            return COLUNA_ABERTA_BONUS;
        }
    }

    return score;
}

int Eval::avaliar(){
    Score score[LADOS] = {0, 0};

    const EntradaPeoes &peoes = estrutura_de_peoes();

    Bitboard::u64 t1;
    int casa;

    const int rei_branco_sq = Bitboard::bitscan(Bitboard::bit_pieces[BRANCAS][R]);
    const int rei_preto_sq  = Bitboard::bitscan(Bitboard::bit_pieces[PRETAS][R]);
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
        const Bitboard::u64 nao_proprios = ~Bitboard::bit_lados[l];
        const Bitboard::u64 zona = zona_inimiga[l];
        int pressao_rei = 0;

        score[l] += score_casas[l][R][rei_sq[l]];

        score[l] += peoes.score[l];

        t1 = Bitboard::bit_pieces[l][C];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][C][casa];
            const Bitboard::u64 att = Gen::bit_moves_cavalo[casa];
            score[l] += mobilidade_cavalo[Bitboard::popcount(att & nao_proprios)];
            pressao_rei += Bitboard::popcount(att & zona) * kw_c;
        }

        t1 = Bitboard::bit_pieces[l][B];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][B][casa];
            const Bitboard::u64 att = Gen::atacantes_bispo(casa);
            score[l] += mobilidade_bispo[Bitboard::popcount(att & nao_proprios)];
            pressao_rei += Bitboard::popcount(att & zona) * kw_b;
        }

        if (Bitboard::popcount(Bitboard::bit_pieces[l][B]) >= 2){
            score[l] += make_score(BISHOP_PAIR_MG, BISHOP_PAIR_EG);
        }

        t1 = Bitboard::bit_pieces[l][T];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][T][casa];
            const int torre_b = avaliar_torre(l, casa);
            score[l] += make_score(torre_b, torre_b);
            const Bitboard::u64 att = Gen::atacantes_torre(casa);
            score[l] += mobilidade_torre[Bitboard::popcount(att & nao_proprios)];
            pressao_rei += Bitboard::popcount(att & zona) * kw_t;
        }

        t1 = Bitboard::bit_pieces[l][D];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][D][casa];
            const Bitboard::u64 att = Gen::atacantes_bispo(casa) | Gen::atacantes_torre(casa);
            score[l] += mobilidade_dama[Bitboard::popcount(att & nao_proprios)];
            pressao_rei += Bitboard::popcount(att & zona) * kw_d;
        }

        score[l] += make_score((pressao_rei * pressao_rei) / KS_SCALE, 0);
    }

    int shield_w = 0;
    if (Bitboard::bit_pieces[BRANCAS][R] & Bitboard::mask_ala_do_rei){
        shield_w = peoes.ala_do_rei[BRANCAS];
    }
    else if (Bitboard::bit_pieces[BRANCAS][R] & Bitboard::mask_ala_da_dama){
        shield_w = peoes.ala_da_dama[BRANCAS];
    }
    score[BRANCAS] += make_score(shield_w, 0);

    int shield_b = 0;
    if (Bitboard::bit_pieces[PRETAS][R] & Bitboard::mask_ala_do_rei){
        shield_b = peoes.ala_do_rei[PRETAS];
    }
    else if (Bitboard::bit_pieces[PRETAS][R] & Bitboard::mask_ala_da_dama){
        shield_b = peoes.ala_da_dama[PRETAS];
    }
    score[PRETAS] += make_score(shield_b, 0);

    const Score diff = score[Game::lado] - score[Game::xlado];
    const int mg_diff = mg_score(diff);
    const int eg_diff = eg_score(diff);
    const int phase = fase();
    return (mg_diff * phase + eg_diff * (PHASE_MAX - phase)) / PHASE_MAX;
}