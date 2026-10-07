// Texel tuner, gradient-based variant.
//
// tuner.cpp (coordinate descent) re-runs Eval::avaliar over the whole dataset
// for every +/-1 probe of every parameter. This tuner exploits the fact that
// the tuned part of the eval is (almost) linear in its parameters:
//
//   eval_white = [ sum_f coef_f * mg_f * fase + sum_f coef_f * eg_f * (MAX-fase) ] / MAX
//              + (PW^2 - PB^2) / KS_SCALE * fase / MAX      (king safety, mg only)
//              + rest                                       (untuned terms, constant)
//
// so each position is reduced ONCE to a sparse feature vector, and every epoch
// is a pass over those vectors with an analytic gradient fed to Adam.
//
// Runs as: ./capi_tuner_adam <dataset.txt> [options]   (see usage below)

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <vector>
#include <algorithm>
#include <random>
#include <limits>
#include <thread>
#include <functional>

#include "consts.h"
#include "init.h"
#include "game.h"
#include "update.h"
#include "bitboard.h"
#include "gen.h"
#include "eval.h"

// ---------------------------------------------------------------------------
// Dataset loading (same format and parsing as tuner.cpp)

struct TuningPos {
    char fen[96];
    char lado[4];
    char roques[6];
    char ep[4];
    char hm[6];
    char fm[6];
    float result;       // 0.0, 0.5, 1.0 (white's point of view)
};

static bool result_to_float(const char *s, float &out) {
    if (!strcmp(s, "1.0") || !strcmp(s, "1") || !strcmp(s, "1-0")) { out = 1.0f; return true; }
    if (!strcmp(s, "0.5") || !strcmp(s, "1/2-1/2") || !strcmp(s, "0.5-0.5")) { out = 0.5f; return true; }
    if (!strcmp(s, "0.0") || !strcmp(s, "0") || !strcmp(s, "0-1")) { out = 0.0f; return true; }
    return false;
}

static bool parse_line(char *line, TuningPos &out) {
    const char *seps = " \t\r\n|[]";
    char *tokens[10] = {0};
    int n = 0;
    char *tok = strtok(line, seps);
    while (tok && n < 10) {
        tokens[n++] = tok;
        tok = strtok(NULL, seps);
    }

    if (n < 5) return false;
    if (!result_to_float(tokens[n - 1], out.result)) return false;

    strncpy(out.fen,    tokens[0], sizeof(out.fen)    - 1); out.fen[sizeof(out.fen)-1] = '\0';
    strncpy(out.lado,   tokens[1], sizeof(out.lado)   - 1); out.lado[sizeof(out.lado)-1] = '\0';
    strncpy(out.roques, tokens[2], sizeof(out.roques) - 1); out.roques[sizeof(out.roques)-1] = '\0';
    strncpy(out.ep,     tokens[3], sizeof(out.ep)     - 1); out.ep[sizeof(out.ep)-1] = '\0';

    if (n >= 7) {
        strncpy(out.hm, tokens[4], sizeof(out.hm) - 1); out.hm[sizeof(out.hm)-1] = '\0';
        strncpy(out.fm, tokens[5], sizeof(out.fm) - 1); out.fm[sizeof(out.fm)-1] = '\0';
    } else {
        strcpy(out.hm, "0");
        strcpy(out.fm, "1");
    }
    return true;
}

static std::vector<TuningPos> load_dataset(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "tuner: cannot open dataset %s: %s\n", path, strerror(errno));
        exit(1);
    }

    std::vector<TuningPos> out;
    char line[512];
    int skipped = 0;
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '\n' || *p == '#') continue;

        TuningPos pos;
        if (parse_line(line, pos)) out.push_back(pos);
        else skipped++;
    }
    fclose(f);

    fprintf(stderr, "tuner: loaded %zu positions, skipped %d\n", out.size(), skipped);
    if (out.empty()) {
        fprintf(stderr, "tuner: empty dataset, aborting\n");
        exit(1);
    }
    return out;
}

// ---------------------------------------------------------------------------
// Parameter layout
//
// Feature f has two parameters (mg, eg): theta[2f], theta[2f+1].
//   f in [0, 384)        PST: piece * 64 + square (white-side indexing)
//   f in [384, 450)      mobility buckets: knight 9, bishop 14, rook 15, queen 28
// theta[KS_BASE + i]      king-safety weights (c, b, t, d), mg only.

static const int PST_FEATURES = 6 * CASAS_DO_TABULEIRO;
static const int MOB_C_BASE = PST_FEATURES;
static const int MOB_B_BASE = MOB_C_BASE + 9;
static const int MOB_T_BASE = MOB_B_BASE + 14;
static const int MOB_D_BASE = MOB_T_BASE + 15;
static const int NUM_FEATURES = MOB_D_BASE + 28;
static const int KS_BASE = 2 * NUM_FEATURES;
static const int NUM_PARAMS = KS_BASE + 4;

// Engine slots behind each feature (white slot, plus the mirrored black slot
// for PSTs). Same aliasing rule as tuner.cpp's Param.
static Eval::Score* feature_slot[NUM_FEATURES];
static Eval::Score* feature_mirror[NUM_FEATURES];
static Eval::Score* ks_slot[4];

static void bind_slots() {
    for (int p = P; p <= R; p++) {
        for (int x = 0; x < CASAS_DO_TABULEIRO; x++) {
            const int f = p * CASAS_DO_TABULEIRO + x;
            feature_slot[f]   = &Eval::score_casas[BRANCAS][p][x];
            feature_mirror[f] = &Eval::score_casas[PRETAS][p][Consts::flip[x]];
        }
    }
    for (int i = 0; i < 9;  i++) { feature_slot[MOB_C_BASE + i] = &Eval::mobilidade_cavalo[i]; feature_mirror[MOB_C_BASE + i] = NULL; }
    for (int i = 0; i < 14; i++) { feature_slot[MOB_B_BASE + i] = &Eval::mobilidade_bispo[i];  feature_mirror[MOB_B_BASE + i] = NULL; }
    for (int i = 0; i < 15; i++) { feature_slot[MOB_T_BASE + i] = &Eval::mobilidade_torre[i];  feature_mirror[MOB_T_BASE + i] = NULL; }
    for (int i = 0; i < 28; i++) { feature_slot[MOB_D_BASE + i] = &Eval::mobilidade_dama[i];   feature_mirror[MOB_D_BASE + i] = NULL; }
    ks_slot[0] = &Eval::ks_weight_c;
    ks_slot[1] = &Eval::ks_weight_b;
    ks_slot[2] = &Eval::ks_weight_t;
    ks_slot[3] = &Eval::ks_weight_d;
}

static void read_theta_from_engine(std::vector<double>& theta) {
    theta.assign(NUM_PARAMS, 0.0);
    for (int f = 0; f < NUM_FEATURES; f++) {
        theta[2 * f]     = Eval::mg_score(*feature_slot[f]);
        theta[2 * f + 1] = Eval::eg_score(*feature_slot[f]);
    }
    for (int i = 0; i < 4; i++) theta[KS_BASE + i] = Eval::mg_score(*ks_slot[i]);
}

static inline int round_i16(double v) {
    long r = lround(v);
    if (r >  32000) r =  32000;
    if (r < -32000) r = -32000;
    return (int)r;
}

static void write_theta_to_engine(const std::vector<double>& theta) {
    for (int f = 0; f < NUM_FEATURES; f++) {
        *feature_slot[f] = Eval::make_score(round_i16(theta[2 * f]), round_i16(theta[2 * f + 1]));
        if (feature_mirror[f]) *feature_mirror[f] = *feature_slot[f];
    }
    for (int i = 0; i < 4; i++) *ks_slot[i] = Eval::make_score(round_i16(theta[KS_BASE + i]), 0);
}

static std::vector<double> rounded(const std::vector<double>& theta) {
    std::vector<double> r(theta.size());
    for (size_t i = 0; i < theta.size(); i++) r[i] = (double)round_i16(theta[i]);
    return r;
}

// ---------------------------------------------------------------------------
// Sparse feature extraction

struct PosInfo {
    float   result;
    float   rest;       // untuned eval terms, white POV, centipawns (already phase-interpolated)
    uint8_t phase;
    uint8_t ks[2][4];   // king-zone attack counts per side (white, black) and piece type (c, b, t, d)
};

struct Data {
    std::vector<PosInfo>  pos;
    std::vector<uint32_t> off;     // pos.size() + 1 entries
    std::vector<uint16_t> idx;
    std::vector<int8_t>   coef;
    size_t size() const { return pos.size(); }
};

struct SavedParams {
    Eval::Score casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
    Eval::Score mc[9], mb[14], mt[15], md[28];
    Eval::Score ks[4];
};

static void save_params(SavedParams& s) {
    memcpy(s.casas, Eval::score_casas, sizeof(s.casas));
    memcpy(s.mc, Eval::mobilidade_cavalo, sizeof(s.mc));
    memcpy(s.mb, Eval::mobilidade_bispo,  sizeof(s.mb));
    memcpy(s.mt, Eval::mobilidade_torre,  sizeof(s.mt));
    memcpy(s.md, Eval::mobilidade_dama,   sizeof(s.md));
    for (int i = 0; i < 4; i++) s.ks[i] = *ks_slot[i];
}

static void restore_params(const SavedParams& s) {
    memcpy(Eval::score_casas, s.casas, sizeof(s.casas));
    memcpy(Eval::mobilidade_cavalo, s.mc, sizeof(s.mc));
    memcpy(Eval::mobilidade_bispo,  s.mb, sizeof(s.mb));
    memcpy(Eval::mobilidade_torre,  s.mt, sizeof(s.mt));
    memcpy(Eval::mobilidade_dama,   s.md, sizeof(s.md));
    for (int i = 0; i < 4; i++) *ks_slot[i] = s.ks[i];
}

static void zero_params() {
    memset(Eval::score_casas, 0, sizeof(Eval::score_casas));
    memset(Eval::mobilidade_cavalo, 0, sizeof(Eval::Score) * 9);
    memset(Eval::mobilidade_bispo,  0, sizeof(Eval::Score) * 14);
    memset(Eval::mobilidade_torre,  0, sizeof(Eval::Score) * 15);
    memset(Eval::mobilidade_dama,   0, sizeof(Eval::Score) * 28);
    for (int i = 0; i < 4; i++) *ks_slot[i] = 0;
}

// Use the evaluator's own zone so the extracted counts always match whatever
// Eval::avaliar() does (see the equivalence check in main).
static inline Bitboard::u64 king_zone_of(int sq) {
    return Eval::zona_do_rei(sq);
}

// Walks the dataset and fills `d`. If `occ` is non-NULL, also counts how many
// times each feature's piece/bucket occurs (the per-parameter "support" that
// tuner.cpp used to freeze rarely-seen parameters).
//
// Must run with the tuned parameters zeroed, so that Eval::avaliar() returns
// exactly the untuned remainder ("rest").
static void extract(const std::vector<TuningPos>& set, Data& d, long* occ) {
    d.pos.clear(); d.off.clear(); d.idx.clear(); d.coef.clear();
    d.pos.reserve(set.size());
    d.off.reserve(set.size() + 1);
    d.idx.reserve(set.size() * 40);
    d.coef.reserve(set.size() * 40);
    d.off.push_back(0);

    int cnt[NUM_FEATURES];

    for (size_t i = 0; i < set.size(); i++) {
        TuningPos p = set[i];  // setar_posicao wants non-const char* fields
        Update::setar_posicao(p.fen, p.lado, p.roques, p.ep, p.hm, p.fm);

        memset(cnt, 0, sizeof(cnt));
        PosInfo info;
        memset(&info, 0, sizeof(info));

        const int rei_sq[LADOS] = {
            Bitboard::bitscan(Bitboard::bit_pieces[BRANCAS][R]),
            Bitboard::bitscan(Bitboard::bit_pieces[PRETAS][R])
        };

        for (int l = 0; l < LADOS; l++) {
            const int sign = (l == BRANCAS) ? 1 : -1;
            const Bitboard::u64 nao_proprios = ~Bitboard::bit_lados[l];
            const Bitboard::u64 zona = king_zone_of(rei_sq[l ^ 1]);

            for (int piece = P; piece <= R; piece++) {
                Bitboard::u64 t = Bitboard::bit_pieces[l][piece];
                while (t) {
                    int casa = Bitboard::bitscan(t);
                    t &= Bitboard::not_mask[casa];
                    int x = (l == BRANCAS) ? casa : Consts::flip[casa];
                    cnt[piece * CASAS_DO_TABULEIRO + x] += sign;
                    if (occ) occ[piece * CASAS_DO_TABULEIRO + x]++;
                }
            }

            Bitboard::u64 t = Bitboard::bit_pieces[l][C];
            while (t) {
                int casa = Bitboard::bitscan(t); t &= Bitboard::not_mask[casa];
                Bitboard::u64 att = Gen::bit_moves_cavalo[casa];
                int f = MOB_C_BASE + Bitboard::popcount(att & nao_proprios);
                cnt[f] += sign; if (occ) occ[f]++;
                info.ks[l][0] += Bitboard::popcount(att & zona);
            }
            t = Bitboard::bit_pieces[l][B];
            while (t) {
                int casa = Bitboard::bitscan(t); t &= Bitboard::not_mask[casa];
                Bitboard::u64 att = Gen::atacantes_bispo(casa);
                int f = MOB_B_BASE + Bitboard::popcount(att & nao_proprios);
                cnt[f] += sign; if (occ) occ[f]++;
                info.ks[l][1] += Bitboard::popcount(att & zona);
            }
            t = Bitboard::bit_pieces[l][T];
            while (t) {
                int casa = Bitboard::bitscan(t); t &= Bitboard::not_mask[casa];
                Bitboard::u64 att = Gen::atacantes_torre(casa);
                int f = MOB_T_BASE + Bitboard::popcount(att & nao_proprios);
                cnt[f] += sign; if (occ) occ[f]++;
                info.ks[l][2] += Bitboard::popcount(att & zona);
            }
            t = Bitboard::bit_pieces[l][D];
            while (t) {
                int casa = Bitboard::bitscan(t); t &= Bitboard::not_mask[casa];
                Bitboard::u64 att = Gen::atacantes_bispo(casa) | Gen::atacantes_torre(casa);
                int f = MOB_D_BASE + Bitboard::popcount(att & nao_proprios);
                cnt[f] += sign; if (occ) occ[f]++;
                info.ks[l][3] += Bitboard::popcount(att & zona);
            }
        }

        info.result = p.result;
        info.phase  = (uint8_t)Eval::fase();

        int e = Eval::avaliar();                  // tuned params are zero: only untuned terms remain
        if (Game::lado == PRETAS) e = -e;         // white POV
        info.rest = (float)e;

        d.pos.push_back(info);
        for (int f = 0; f < NUM_FEATURES; f++) {
            if (cnt[f] != 0) {
                d.idx.push_back((uint16_t)f);
                d.coef.push_back((int8_t)cnt[f]);
            }
        }
        d.off.push_back((uint32_t)d.idx.size());
    }
}

// ---------------------------------------------------------------------------
// Model: loss and gradient

static inline double sigmoid(double cp, double K) {
    return 1.0 / (1.0 + std::pow(10.0, -K * cp / 400.0));
}

static inline double eval_pos(const Data& d, size_t i, const double* th) {
    double mg = 0.0, eg = 0.0;
    for (uint32_t k = d.off[i]; k < d.off[i + 1]; k++) {
        const int f = d.idx[k];
        const double c = d.coef[k];
        mg += c * th[2 * f];
        eg += c * th[2 * f + 1];
    }
    const PosInfo& pi = d.pos[i];
    double pw = 0.0, pb = 0.0;
    for (int j = 0; j < 4; j++) {
        pw += pi.ks[0][j] * th[KS_BASE + j];
        pb += pi.ks[1][j] * th[KS_BASE + j];
    }
    mg += (pw * pw - pb * pb) / KS_SCALE;
    return (mg * pi.phase + eg * (PHASE_MAX - pi.phase)) / PHASE_MAX + pi.rest;
}

// Adds sum of squared errors over [b, e) to *loss and, if grad != NULL, the
// gradient of that sum w.r.t. every parameter.
static void range_loss_grad(const Data& d, size_t b, size_t e, const double* th, double K,
                            double* loss, double* grad) {
    const double kconst = K * std::log(10.0) / 400.0;
    double sum = 0.0;
    for (size_t i = b; i < e; i++) {
        const double ev = eval_pos(d, i, th);
        const double s = sigmoid(ev, K);
        const double err = s - d.pos[i].result;
        sum += err * err;
        if (!grad) continue;

        const PosInfo& pi = d.pos[i];
        const double g = 2.0 * err * s * (1.0 - s) * kconst;   // dLoss/dEval
        const double wmg = g * pi.phase / PHASE_MAX;
        const double weg = g * (PHASE_MAX - pi.phase) / PHASE_MAX;

        for (uint32_t k = d.off[i]; k < d.off[i + 1]; k++) {
            const int f = d.idx[k];
            const double c = d.coef[k];
            grad[2 * f]     += wmg * c;
            grad[2 * f + 1] += weg * c;
        }
        double pw = 0.0, pb = 0.0;
        for (int j = 0; j < 4; j++) {
            pw += pi.ks[0][j] * th[KS_BASE + j];
            pb += pi.ks[1][j] * th[KS_BASE + j];
        }
        for (int j = 0; j < 4; j++) {
            grad[KS_BASE + j] += wmg * 2.0 * (pw * pi.ks[0][j] - pb * pi.ks[1][j]) / KS_SCALE;
        }
    }
    *loss += sum;
}

static int num_threads = 0;

// Full-dataset mean loss (and gradient if `grad` != NULL). Each thread owns a
// fixed slice and a private gradient buffer; buffers are reduced in thread
// order, so results are deterministic for a given thread count.
static double mean_loss_grad(const Data& d, const std::vector<double>& theta, double K,
                             std::vector<double>* grad) {
    const size_t n = d.size();
    const int nt = (int)std::max<size_t>(1, std::min<size_t>((size_t)num_threads, n / 1024 + 1));

    std::vector<double> losses(nt, 0.0);
    std::vector<std::vector<double> > grads;
    if (grad) grads.assign(nt, std::vector<double>(NUM_PARAMS, 0.0));

    std::vector<std::thread> workers;
    for (int t = 0; t < nt; t++) {
        const size_t b = n * t / nt, e = n * (t + 1) / nt;
        double* gp = grad ? &grads[t][0] : NULL;
        if (t == nt - 1) {
            range_loss_grad(d, b, e, &theta[0], K, &losses[t], gp);
        } else {
            workers.push_back(std::thread(range_loss_grad, std::cref(d), b, e, &theta[0], K, &losses[t], gp));
        }
    }
    for (size_t i = 0; i < workers.size(); i++) workers[i].join();

    double total = 0.0;
    for (int t = 0; t < nt; t++) total += losses[t];
    if (grad) {
        grad->assign(NUM_PARAMS, 0.0);
        for (int t = 0; t < nt; t++)
            for (int p = 0; p < NUM_PARAMS; p++) (*grad)[p] += grads[t][p];
        for (int p = 0; p < NUM_PARAMS; p++) (*grad)[p] /= (double)n;
    }
    return total / (double)n;
}

static double optimize_K(const Data& d, const std::vector<double>& theta) {
    double low = 0.1, high = 3.0;
    for (int iter = 0; iter < 40 && (high - low) > 0.0001; iter++) {
        double m1 = low + (high - low) / 3.0;
        double m2 = high - (high - low) / 3.0;
        double l1 = mean_loss_grad(d, theta, m1, NULL);
        double l2 = mean_loss_grad(d, theta, m2, NULL);
        if (l1 < l2) high = m2; else low = m1;
    }
    return (low + high) / 2.0;
}

// ---------------------------------------------------------------------------
// Sanity checks

// Rebuilds the eval from features + theta and compares to the real engine.
static void check_equivalence(const std::vector<TuningPos>& set, const Data& d,
                              const std::vector<double>& theta, size_t n) {
    n = std::min(n, set.size());
    double max_diff = 0.0, sum_diff = 0.0;
    size_t worst = 0;
    for (size_t i = 0; i < n; i++) {
        TuningPos p = set[i];
        Update::setar_posicao(p.fen, p.lado, p.roques, p.ep, p.hm, p.fm);
        int e = Eval::avaliar();
        if (Game::lado == PRETAS) e = -e;
        double diff = std::fabs(eval_pos(d, i, &theta[0]) - (double)e);
        sum_diff += diff;
        if (diff > max_diff) { max_diff = diff; worst = i; }
    }
    fprintf(stderr, "tuner: equivalence vs Eval::avaliar over %zu positions: max |diff| = %.2f cp (pos %zu), mean = %.3f cp\n",
            n, max_diff, worst, sum_diff / (double)std::max<size_t>(1, n));
    if (max_diff > 4.0) {
        fprintf(stderr, "tuner: ERROR: sparse evaluator disagrees with the engine eval; aborting\n");
        exit(2);
    }
}

// Mean loss measured through the real engine (Eval::avaliar) with `theta`
// rounded into the engine slots. Independent of the sparse model.
static double engine_loss(const std::vector<TuningPos>& set, size_t n, double K,
                          const std::vector<double>& theta) {
    n = std::min(n, set.size());
    write_theta_to_engine(theta);
    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        TuningPos p = set[i];
        Update::setar_posicao(p.fen, p.lado, p.roques, p.ep, p.hm, p.fm);
        int e = Eval::avaliar();
        if (Game::lado == PRETAS) e = -e;
        double err = (double)p.result - sigmoid((double)e, K);
        sum += err * err;
    }
    return sum / (double)n;
}

static void gradient_check(const Data& d, const std::vector<double>& theta, double K) {
    std::vector<double> grad;
    mean_loss_grad(d, theta, K, &grad);

    fprintf(stderr, "tuner: gradient check (analytic vs central difference)\n");
    double worst = 0.0;
    std::vector<int> probe;
    for (int p = 0; p < KS_BASE; p += 37) probe.push_back(p);   // PST + mobility, mg and eg
    for (int i = 0; i < 4; i++) probe.push_back(KS_BASE + i);   // king-safety weights
    for (size_t pi = 0; pi < probe.size(); pi++) {
        const int p = probe[pi];
        const double h = 1e-3;
        std::vector<double> tp = theta, tm = theta;
        tp[p] += h; tm[p] -= h;
        double num = (mean_loss_grad(d, tp, K, NULL) - mean_loss_grad(d, tm, K, NULL)) / (2.0 * h);
        double denom = std::max(1e-12, std::fabs(num) + std::fabs(grad[p]));
        double rel = std::fabs(num - grad[p]) / denom;
        if (rel > worst) worst = rel;
        fprintf(stderr, "  param %4d: analytic % .6e  numeric % .6e  rel.err %.2e\n", p, grad[p], num, rel);
    }
    fprintf(stderr, "tuner: gradient check worst relative error = %.2e\n", worst);

    // The king-safety gradient is identically zero whenever the dataset has no
    // king-zone attacks (true while the evaluator's zone is degenerate), which
    // would make the check above vacuous for those 4 weights. Re-check them on
    // a copy with synthetic attack counts.
    Data fake = d;
    std::mt19937 rng(7);
    for (size_t i = 0; i < fake.pos.size(); i++)
        for (int l = 0; l < 2; l++)
            for (int j = 0; j < 4; j++) fake.pos[i].ks[l][j] = (uint8_t)(rng() % 4);
    mean_loss_grad(fake, theta, K, &grad);
    double worst_ks = 0.0;
    for (int i = 0; i < 4; i++) {
        const int p = KS_BASE + i;
        const double h = 1e-3;
        std::vector<double> tp = theta, tm = theta;
        tp[p] += h; tm[p] -= h;
        double num = (mean_loss_grad(fake, tp, K, NULL) - mean_loss_grad(fake, tm, K, NULL)) / (2.0 * h);
        double rel = std::fabs(num - grad[p]) / std::max(1e-12, std::fabs(num) + std::fabs(grad[p]));
        if (rel > worst_ks) worst_ks = rel;
        fprintf(stderr, "  ks    %4d: analytic % .6e  numeric % .6e  rel.err %.2e  (synthetic counts)\n", p, grad[p], num, rel);
    }
    fprintf(stderr, "tuner: king-safety gradient worst relative error = %.2e\n", worst_ks);
}

// ---------------------------------------------------------------------------
// Output (same format as tuner.cpp)

static void print_pst_array(FILE* out, const char* name, int piece, bool is_mg) {
    fprintf(out, "\tconst int %s[64] = {\n", name);
    for (int rank = 0; rank < 8; rank++) {
        fprintf(out, "\t\t");
        for (int file = 0; file < 8; file++) {
            int x = rank * 8 + file;
            int v = is_mg ? Eval::mg_score(Eval::score_casas[BRANCAS][piece][x])
                          : Eval::eg_score(Eval::score_casas[BRANCAS][piece][x]);
            fprintf(out, "%5d%s", v, (file == 7 && rank == 7) ? "\n" : ",");
            if (file == 7 && rank != 7) fprintf(out, "\n");
        }
    }
    fprintf(out, "\t};\n\n");
}

static void print_mobility_array(FILE* out, const char* name, const Eval::Score* tbl, int len, bool is_mg) {
    fprintf(out, "\tconst int %s[%d] = {\n\t\t", name, len);
    for (int i = 0; i < len; i++) {
        int v = is_mg ? Eval::mg_score(tbl[i]) : Eval::eg_score(tbl[i]);
        fprintf(out, "%5d%s", v, (i == len - 1) ? "\n" : ",");
        if ((i + 1) % 8 == 0 && i != len - 1) fprintf(out, "\n\t\t");
    }
    fprintf(out, "\t};\n\n");
}

static void print_tuned_values(FILE* out) {
    fprintf(out, "// ==== TUNED VALUES (paste into values.h, set VALOR_*_MG/_EG to 0) ====\n");
    fprintf(out, "// PSTs below have material baked in.\n\n");

    print_pst_array(out, "peao_score_mg",   P, true);
    print_pst_array(out, "peao_score_eg",   P, false);
    print_pst_array(out, "cavalo_score_mg", C, true);
    print_pst_array(out, "cavalo_score_eg", C, false);
    print_pst_array(out, "bispo_score_mg",  B, true);
    print_pst_array(out, "bispo_score_eg",  B, false);
    print_pst_array(out, "torre_score_mg",  T, true);
    print_pst_array(out, "torre_score_eg",  T, false);
    print_pst_array(out, "dama_score_mg",   D, true);
    print_pst_array(out, "dama_score_eg",   D, false);
    print_pst_array(out, "rei_score_mg",    R, true);
    print_pst_array(out, "rei_score_eg",    R, false);

    print_mobility_array(out, "mobilidade_cavalo_mg", Eval::mobilidade_cavalo, 9,  true);
    print_mobility_array(out, "mobilidade_cavalo_eg", Eval::mobilidade_cavalo, 9,  false);
    print_mobility_array(out, "mobilidade_bispo_mg",  Eval::mobilidade_bispo,  14, true);
    print_mobility_array(out, "mobilidade_bispo_eg",  Eval::mobilidade_bispo,  14, false);
    print_mobility_array(out, "mobilidade_torre_mg",  Eval::mobilidade_torre,  15, true);
    print_mobility_array(out, "mobilidade_torre_eg",  Eval::mobilidade_torre,  15, false);
    print_mobility_array(out, "mobilidade_dama_mg",   Eval::mobilidade_dama,   28, true);
    print_mobility_array(out, "mobilidade_dama_eg",   Eval::mobilidade_dama,   28, false);

    fprintf(out, "\t#define KS_WEIGHT_C %d\n",   Eval::mg_score(Eval::ks_weight_c));
    fprintf(out, "\t#define KS_WEIGHT_B %d\n",   Eval::mg_score(Eval::ks_weight_b));
    fprintf(out, "\t#define KS_WEIGHT_T %d\n",   Eval::mg_score(Eval::ks_weight_t));
    fprintf(out, "\t#define KS_WEIGHT_D %d\n\n", Eval::mg_score(Eval::ks_weight_d));

    fprintf(out, "// ==== end tuned values ====\n");
}

static const char* checkpoint_path = NULL;

static void write_checkpoint(int epoch, double train_loss, double val_loss,
                             const std::vector<double>& theta) {
    if (!checkpoint_path) return;

    write_theta_to_engine(theta);

    char tmp_path[1024];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", checkpoint_path);
    FILE* f = fopen(tmp_path, "w");
    if (!f) {
        fprintf(stderr, "tuner: cannot write checkpoint %s: %s\n", tmp_path, strerror(errno));
        return;
    }
    fprintf(f, "// checkpoint: best validation loss=%.6f (train=%.6f) at epoch %d\n",
            val_loss, train_loss, epoch);
    print_tuned_values(f);
    fclose(f);
    if (rename(tmp_path, checkpoint_path) != 0)
        fprintf(stderr, "tuner: cannot rename checkpoint to %s: %s\n", checkpoint_path, strerror(errno));
}

// ---------------------------------------------------------------------------
// Optimizers

struct OptimizerCfg {
    bool   adagrad;
    double lr;
    int    epochs;
    int    patience;
};

static void train(const Data& train_d, const Data& val_d, double K,
                  std::vector<double>& theta, const std::vector<char>& frozen,
                  const OptimizerCfg& cfg) {
    const double b1 = 0.9, b2 = 0.999, eps = 1e-8;
    std::vector<double> m(NUM_PARAMS, 0.0), v(NUM_PARAMS, 0.0), grad;

    double best_val = std::numeric_limits<double>::max();
    std::vector<double> best_theta = theta;
    int since_best = 0, last_ckpt = -1000;
    const double PI = 3.14159265358979323846;

    for (int ep = 1; ep <= cfg.epochs; ep++) {
        double train_loss = mean_loss_grad(train_d, theta, K, &grad);

        // cosine decay to 2% of the base learning rate
        const double frac = (double)(ep - 1) / (double)std::max(1, cfg.epochs - 1);
        const double lr = cfg.lr * (0.02 + 0.98 * 0.5 * (1.0 + std::cos(PI * frac)));

        for (int p = 0; p < NUM_PARAMS; p++) {
            if (frozen[p]) continue;
            const double g = grad[p];
            if (cfg.adagrad) {
                v[p] += g * g;
                theta[p] -= lr * g / (std::sqrt(v[p]) + eps);
            } else {
                m[p] = b1 * m[p] + (1.0 - b1) * g;
                v[p] = b2 * v[p] + (1.0 - b2) * g * g;
                const double mh = m[p] / (1.0 - std::pow(b1, ep));
                const double vh = v[p] / (1.0 - std::pow(b2, ep));
                theta[p] -= lr * mh / (std::sqrt(vh) + eps);
            }
        }

        double val_loss = val_d.size() ? mean_loss_grad(val_d, theta, K, NULL) : train_loss;
        bool improved = val_loss < best_val;
        if (improved) {
            best_val = val_loss;
            best_theta = theta;
            since_best = 0;
            if (ep - last_ckpt >= 50) { write_checkpoint(ep, train_loss, val_loss, rounded(theta)); last_ckpt = ep; }
        } else {
            since_best++;
        }

        if (ep == 1 || ep % 10 == 0)
            fprintf(stderr, "tuner: epoch %4d  lr=%.4f  loss=%.6f (train)  %.6f (val)%s\n",
                    ep, lr, train_loss, val_loss, improved ? " *" : "");

        if (cfg.patience > 0 && since_best >= cfg.patience) {
            fprintf(stderr, "tuner: stopping at epoch %d (validation hasn't improved in %d epochs)\n", ep, cfg.patience);
            break;
        }
    }

    theta = best_theta;
    fprintf(stderr, "tuner: restored best-validation parameters (val loss=%.6f)\n", best_val);
}

// ---------------------------------------------------------------------------
// Entry point

static void usage(const char* prog) {
    fprintf(stderr, "Usage: %s <dataset.txt> [options]\n", prog);
    fprintf(stderr, "  --max N          Cap dataset to N positions (random shuffle then truncate). Default: 0 = all.\n");
    fprintf(stderr, "  --epochs E       Full-batch epochs. Default: 1000.\n");
    fprintf(stderr, "  --lr X           Base learning rate in centipawns per step. Default: 1.0 (cosine-decayed to 2%%).\n");
    fprintf(stderr, "  --opt NAME       adam | adagrad. Default: adam.\n");
    fprintf(stderr, "  --threads N      Worker threads for loss/gradient. Default: hardware concurrency.\n");
    fprintf(stderr, "  --checkpoint F   Rewrite F with the best-validation tuned values periodically.\n");
    fprintf(stderr, "  --val-frac F     Fraction held out for validation. Default: 0.15.\n");
    fprintf(stderr, "  --min-samples N  Freeze parameters touched by fewer than N training occurrences. Default: 200.\n");
    fprintf(stderr, "  --patience P     Stop after P epochs without validation improvement. Default: 100. 0 = disabled.\n");
    fprintf(stderr, "  --gradcheck      Compare analytic gradient to finite differences, then exit.\n");
}

int main(int argc, char **argv) {
    if (argc < 2) { usage(argv[0]); return 1; }

    int max_positions = 0;
    double val_frac   = 0.15;
    long min_samples  = 200;
    bool do_gradcheck = false;
    OptimizerCfg cfg;
    cfg.adagrad = false; cfg.lr = 1.0; cfg.epochs = 1000; cfg.patience = 100;
    num_threads = (int)std::thread::hardware_concurrency();
    if (num_threads < 1) num_threads = 1;

    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--max") && i + 1 < argc)              max_positions = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--epochs") && i + 1 < argc)      cfg.epochs = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--lr") && i + 1 < argc)          cfg.lr = atof(argv[++i]);
        else if (!strcmp(argv[i], "--opt") && i + 1 < argc)         cfg.adagrad = !strcmp(argv[++i], "adagrad");
        else if (!strcmp(argv[i], "--threads") && i + 1 < argc)     num_threads = std::max(1, atoi(argv[++i]));
        else if (!strcmp(argv[i], "--checkpoint") && i + 1 < argc)  checkpoint_path = argv[++i];
        else if (!strcmp(argv[i], "--val-frac") && i + 1 < argc)    val_frac = atof(argv[++i]);
        else if (!strcmp(argv[i], "--min-samples") && i + 1 < argc) min_samples = atol(argv[++i]);
        else if (!strcmp(argv[i], "--patience") && i + 1 < argc)    cfg.patience = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--gradcheck"))                   do_gradcheck = true;
        else { fprintf(stderr, "tuner: unknown option %s\n", argv[i]); usage(argv[0]); return 1; }
    }

    fprintf(stderr, "tuner: initializing engine...\n");
    Init::init();
    bind_slots();

    std::vector<TuningPos> all = load_dataset(argv[1]);

    if (max_positions > 0 && (int)all.size() > max_positions) {
        std::mt19937 rng(0xDA7A);
        std::shuffle(all.begin(), all.end(), rng);
        all.resize(max_positions);
        fprintf(stderr, "tuner: subsampled to %d positions\n", max_positions);
    }

    std::vector<TuningPos> val_set;
    if (val_frac > 0.0 && val_frac < 1.0) {
        std::mt19937 rng(0xFEED);
        std::shuffle(all.begin(), all.end(), rng);
        size_t val_n = (size_t)(all.size() * val_frac);
        val_set.assign(all.end() - val_n, all.end());
        all.resize(all.size() - val_n);
    }
    fprintf(stderr, "tuner: split into %zu training / %zu validation positions, %d thread(s)\n",
            all.size(), val_set.size(), num_threads);

    // Starting point = whatever the engine was compiled with.
    std::vector<double> theta;
    read_theta_from_engine(theta);
    const std::vector<double> theta0 = theta;
    SavedParams saved;
    save_params(saved);

    // Extraction needs the tuned parameters zeroed (see extract()).
    fprintf(stderr, "tuner: extracting sparse features (one pass)...\n");
    time_t t0 = time(NULL);
    long occ[NUM_FEATURES];
    memset(occ, 0, sizeof(occ));
    Data train_d, val_d;
    zero_params();
    extract(all, train_d, occ);
    extract(val_set, val_d, NULL);
    restore_params(saved);
    fprintf(stderr, "tuner: extracted %zu + %zu positions, %zu training nnz (%.1f/pos), %lds\n",
            train_d.size(), val_d.size(), train_d.idx.size(),
            (double)train_d.idx.size() / (double)train_d.size(), (long)(time(NULL) - t0));

    check_equivalence(all, train_d, theta0, 5000);

    double K = optimize_K(train_d, theta0);
    fprintf(stderr, "tuner: optimal K = %.4f\n", K);

    if (do_gradcheck) {
        gradient_check(train_d, theta0, K);
        return 0;
    }

    std::vector<char> frozen(NUM_PARAMS, 0);
    int nfrozen = 0;
    for (int f = 0; f < NUM_FEATURES; f++) {
        if (occ[f] < min_samples) { frozen[2 * f] = frozen[2 * f + 1] = 1; nfrozen += 2; }
    }
    fprintf(stderr, "tuner: %d/%d parameters frozen (support < %ld training occurrences)\n",
            nfrozen, NUM_PARAMS, min_samples);

    const double init_train = mean_loss_grad(train_d, theta, K, NULL);
    const double init_val   = val_d.size() ? mean_loss_grad(val_d, theta, K, NULL) : init_train;
    fprintf(stderr, "tuner: starting loss = %.6f (train), %.6f (val)\n", init_train, init_val);

    time_t t1 = time(NULL);
    train(train_d, val_d, K, theta, frozen, cfg);
    fprintf(stderr, "tuner: optimization took %lds\n", (long)(time(NULL) - t1));

    // Final numbers are for the integers that will actually ship.
    const std::vector<double> final_theta = rounded(theta);
    const double fin_train = mean_loss_grad(train_d, final_theta, K, NULL);
    const double fin_val   = val_d.size() ? mean_loss_grad(val_d, final_theta, K, NULL) : fin_train;
    fprintf(stderr, "tuner: final loss (rounded) = %.6f (train), %.6f (val)\n", fin_train, fin_val);

    // Independent confirmation through the real eval on a validation slice.
    if (!val_set.empty()) {
        const size_t n = std::min<size_t>(val_set.size(), 20000);
        Data slice_d;
        std::vector<TuningPos> slice(val_set.begin(), val_set.begin() + n);
        zero_params();
        extract(slice, slice_d, NULL);
        const double sparse_l = mean_loss_grad(slice_d, final_theta, K, NULL);
        const double engine_l = engine_loss(slice, n, K, final_theta);
        fprintf(stderr, "tuner: engine cross-check on %zu val positions: sparse loss %.6f vs Eval::avaliar loss %.6f\n",
                n, sparse_l, engine_l);
    }

    write_theta_to_engine(final_theta);
    if (checkpoint_path) write_checkpoint(cfg.epochs, fin_train, fin_val, final_theta);
    print_tuned_values(stdout);
    return 0;
}
