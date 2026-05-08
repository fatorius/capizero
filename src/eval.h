#ifndef EVAL
#define EVAL

#include <cstdint>

#include "consts.h"

namespace Eval{
    // Packed midgame/endgame score (Stockfish-style). eg lives in the high
    // 16 bits, mg in the low 16. Construction uses + (not |) and the eg
    // extractor adds 0x8000 to round-correct the carry that mg's negative
    // sign-extension introduces into the eg half during +. With this
    // encoding, plain int32 addition of two Scores correctly sums both mg
    // and eg as if computed independently (verified algebraically: the
    // borrow from negative mg cancels symmetrically when both operands
    // share the same encoding). Per-half range stays signed int16.
    typedef int32_t Score;

    inline Score make_score(int mg, int eg){
        return (Score)(((uint32_t)eg << 16) + (uint32_t)mg);
    }
    // mg extraction is a clean low-16 mask + sign extension.
    inline int mg_score(Score s){
        return (int16_t)((uint32_t)s & 0xFFFF);
    }
    // eg extraction adds 0x8000 first so the high-16 read rounds up by one
    // when mg has its sign bit set (i.e. mg < 0), undoing the borrow that
    // the construction `+` left in the high half.
    inline int eg_score(Score s){
        return (int16_t)(((uint32_t)s + 0x8000) >> 16);
    }

    // Single packed PST + material table replacing the previous mg/eg pair.
    // Memory footprint stays the same (one int32 per slot vs two int32s), so
    // cache behavior is comparable, but every per-piece accumulation in
    // `avaliar` becomes one load + one add instead of two of each.
    extern Score score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
    extern int passados[LADOS][CASAS_DO_TABULEIRO];

    extern int peao_mat[2];
    extern int piece_mat[2];

    // Incremental phase counter: maintained by Update::adicionar_piece /
    // Update::remover_piece (one += / -= per call) instead of recomputed
    // on every Eval::avaliar. Reset to 0 at each board-init path
    // (Bitboard::init_board, Game::nova_posicao, Update::setar_posicao);
    // those routines already loop through adicionar_piece per square, so
    // the counter naturally accumulates the full position's phase weight.
    // Like peao_mat / piece_mat / peao_ala_*, this is currently a global —
    // when Lazy SMP arrives (§15 item 19) all of them migrate to per-thread
    // state together.
    extern int fase_valor;
    extern const int phase_weights[6];

    void init_eval_tables();
    void atualizar_materiais();

    // Cheap accessor: clamps fase_valor to [0, PHASE_MAX]. Promotion to a
    // 9th queen could push the raw counter past PHASE_MAX, so the clamp
    // keeps the interpolation in its valid range.
    int fase();

    int avaliar();
};

#endif