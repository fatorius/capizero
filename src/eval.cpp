#include "eval.h"

#include "bitboard.h"
#include "consts.h"
#include "game.h"
#include "gen.h"


Eval::Score Eval::score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
int Eval::passados[LADOS][CASAS_DO_TABULEIRO];

int Eval::peao_mat[LADOS];
int Eval::piece_mat[LADOS];

int Eval::fase_valor = 0;

// Phase weight per piece type, indexed by the {P, C, B, T, D, R} encoding
// in consts.h. Mirrors Values::PHASE_* macros — kept here as a contiguous
// array so Update::adicionar_piece / Update::remover_piece can do a single
// indexed load instead of a switch.
const int Eval::phase_weights[6] = {
    PHASE_PEAO, PHASE_CAVALO, PHASE_BISPO, PHASE_TORRE, PHASE_DAMA, PHASE_REI
};

// Mobility tables. Definitions are storage-only; the values live in
// `Values::mobilidade_*_mg / _eg` in values.h and get packed into Score
// pairs at startup by init_eval_tables. Same pattern capizero uses for
// PSTs (raw values in Values::, packed/initialized in init_eval_tables).
Eval::Score Eval::mobilidade_cavalo[9];
Eval::Score Eval::mobilidade_bispo[14];
Eval::Score Eval::mobilidade_torre[15];
Eval::Score Eval::mobilidade_dama[28];

int peao_ala_da_dama[LADOS],peao_ala_do_rei[LADOS];

void Eval::init_eval_tables(){
    // Phase 1 of the tapered-eval rollout still has mg = eg, so the packed
    // score per square just duplicates the legacy single value into both
    // halves. Future passes will differentiate (e.g. king PST: mg = rei_score,
    // eg = rei_finais_score, dropping the conditional swap below).
    for (int x = 0; x < CASAS_DO_TABULEIRO; x++){
        const int peao_w   = Values::peao_score[x] + VALOR_PEAO;
        const int cavalo_w = Values::cavalo_score[x] + VALOR_CAVALO;
        const int bispo_w  = Values::bispo_score[x] + VALOR_BISPO;
        const int torre_w  = Values::torre_score[x] + VALOR_TORRE;
        const int dama_w   = Values::dama_score[x] + VALOR_DAMA;
        const int rei_w    = Values::rei_score[x];

        // King PST is the first natively-tapered term: mg = rei_score
        // (cares about safety / the back rank), eg = rei_finais_score
        // (cares about activity / centralization). The legacy code applied
        // a hard conditional swap based on enemy-queen presence; tapered
        // eval replaces that with smooth phase interpolation.
        const int rei_w_eg = Values::rei_finais_score[x];
        const int rei_b_eg = Values::rei_finais_score[Consts::flip[x]];

        score_casas[BRANCAS][P][x] = make_score(peao_w,   peao_w);
        score_casas[BRANCAS][C][x] = make_score(cavalo_w, cavalo_w);
        score_casas[BRANCAS][B][x] = make_score(bispo_w,  bispo_w);
        score_casas[BRANCAS][T][x] = make_score(torre_w,  torre_w);
        score_casas[BRANCAS][D][x] = make_score(dama_w,   dama_w);
        score_casas[BRANCAS][R][x] = make_score(rei_w,    rei_w_eg);

        const int peao_b   = Values::peao_score[Consts::flip[x]] + VALOR_PEAO;
        const int cavalo_b = Values::cavalo_score[Consts::flip[x]] + VALOR_CAVALO;
        const int bispo_b  = Values::bispo_score[Consts::flip[x]] + VALOR_BISPO;
        const int torre_b  = Values::torre_score[Consts::flip[x]] + VALOR_TORRE;
        const int dama_b   = Values::dama_score[Consts::flip[x]] + VALOR_DAMA;
        const int rei_b    = Values::rei_score[Consts::flip[x]];

        score_casas[PRETAS][P][x] = make_score(peao_b,   peao_b);
        score_casas[PRETAS][C][x] = make_score(cavalo_b, cavalo_b);
        score_casas[PRETAS][B][x] = make_score(bispo_b,  bispo_b);
        score_casas[PRETAS][T][x] = make_score(torre_b,  torre_b);
        score_casas[PRETAS][D][x] = make_score(dama_b,   dama_b);
        score_casas[PRETAS][R][x] = make_score(rei_b,    rei_b_eg);

        passados[BRANCAS][x] = Values::peao_passado_score[Consts::flip[x]];
        passados[PRETAS][x] = Values::peao_passado_score[x];
    }

    // Pack mobility tables. Values live in values.h as parallel mg/eg
    // arrays; build the packed Score lookup once at startup.
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

int avaliar_peao(const int l, const int casa){
    int score = 0;
    int xl = l^1;

    if (!(Bitboard::mask_passados[l][casa] & Bitboard::bit_pieces[xl][P]) && !(Bitboard::mask_path[l][casa] & Bitboard::bit_pieces[l][P])){
        score += Eval::passados[l][casa];
    }

    if ((Bitboard::mask_isolados[casa] & Bitboard::bit_pieces[l][P]) == 0){
        score -= ISOLADO_SCORE;
    }

    peao_ala_do_rei[l] = Values::defesa_ala_do_rei[l][casa];
    peao_ala_da_dama[l] = Values::defesa_ala_da_dama[l][casa];

    return score;
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
    // Single packed accumulator per side. mg lives in the high 16 bits,
    // eg in the low 16; both halves accumulate in lockstep through plain
    // int32 addition. Single-valued bonuses (avaliar_peao, avaliar_torre,
    // king PST swap, pawn shield) are wrapped via make_score(v, v) so the
    // same delta lands in both halves. Future tapered terms differentiate
    // by passing distinct mg/eg values to make_score.
    Score score[LADOS] = {0, 0};

    peao_ala_da_dama[BRANCAS] = 0;
    peao_ala_do_rei[BRANCAS] = 0;
    peao_ala_da_dama[PRETAS] = 0;
    peao_ala_do_rei[PRETAS] = 0;

    Bitboard::u64 t1;
    int casa;

    for (int l = 0; l < LADOS; l++){

        // Mobility uses popcount of attack squares that aren't own pieces.
        // Each attacked square counts; enemy-occupied squares (capturable
        // targets) and empty squares both contribute. Phase-aware weights
        // recognize that bishop/rook mobility matters more in the endgame
        // where boards are open, while knight mobility is roughly uniform.
        const Bitboard::u64 nao_proprios = ~Bitboard::bit_lados[l];

        t1 = Bitboard::bit_pieces[l][P];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][P][casa];
            const int peao_b = avaliar_peao(l, casa);
            score[l] += make_score(peao_b, peao_b);
        }

        t1 = Bitboard::bit_pieces[l][C];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][C][casa];
            score[l] += mobilidade_cavalo[Bitboard::popcount(Gen::bit_moves_cavalo[casa] & nao_proprios)];
        }

        t1 = Bitboard::bit_pieces[l][B];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][B][casa];
            score[l] += mobilidade_bispo[Bitboard::popcount(Gen::atacantes_bispo(casa) & nao_proprios)];
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
            score[l] += mobilidade_torre[Bitboard::popcount(Gen::atacantes_torre(casa) & nao_proprios)];
        }

        t1 = Bitboard::bit_pieces[l][D];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][D][casa];
            score[l] += mobilidade_dama[Bitboard::popcount((Gen::atacantes_bispo(casa) | Gen::atacantes_torre(casa)) & nao_proprios)];
        }
    }

    // Pawn shield is a midgame-only concept (king wants to stay tucked
    // behind pawns while the opponent has heavy pieces; in the endgame
    // the king should be active). The legacy code gated this on
    // `enemy_queen_present` as a binary proxy for "still mg"; with the
    // king PST now natively tapered, we just apply the shield to the mg
    // half and let the phase interpolation fade it out as material thins.
    int shield_w = 0;
    if (Bitboard::bit_pieces[BRANCAS][R] & Bitboard::mask_ala_do_rei){
        shield_w = peao_ala_do_rei[BRANCAS];
    }
    else if (Bitboard::bit_pieces[BRANCAS][R] & Bitboard::mask_ala_da_dama){
        shield_w = peao_ala_da_dama[BRANCAS];
    }
    score[BRANCAS] += make_score(shield_w, 0);

    int shield_b = 0;
    if (Bitboard::bit_pieces[PRETAS][R] & Bitboard::mask_ala_do_rei){
        shield_b = peao_ala_do_rei[PRETAS];
    }
    else if (Bitboard::bit_pieces[PRETAS][R] & Bitboard::mask_ala_da_dama){
        shield_b = peao_ala_da_dama[PRETAS];
    }
    score[PRETAS] += make_score(shield_b, 0);

    // Side-to-move difference, then unpack and interpolate.
    const Score diff = score[Game::lado] - score[Game::xlado];
    const int mg_diff = mg_score(diff);
    const int eg_diff = eg_score(diff);
    const int phase = fase();
    return (mg_diff * phase + eg_diff * (PHASE_MAX - phase)) / PHASE_MAX;
}