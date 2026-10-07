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

#include "consts.h"
#include "init.h"
#include "game.h"
#include "update.h"
#include "bitboard.h"
#include "gen.h"
#include "eval.h"

struct TuningPos {
    char fen[96];
    char lado[4];
    char roques[6];
    char ep[4];
    char hm[6];
    char fm[6];
    float result;       // 0.0, 0.5, 1.0
};

static std::vector<TuningPos> dataset;  // training set (validation slice is carved out in main)

static bool result_to_float(const char *s, float &out) {
    if (!strcmp(s, "1.0") || !strcmp(s, "1") || !strcmp(s, "1-0")) {
        out = 1.0f; return true;
    }
    if (!strcmp(s, "0.5") || !strcmp(s, "1/2-1/2") || !strcmp(s, "0.5-0.5")) {
        out = 0.5f; return true;
    }
    if (!strcmp(s, "0.0") || !strcmp(s, "0") || !strcmp(s, "0-1")) {
        out = 0.0f; return true;
    }
    return false;
}

static bool parse_line(char *line, TuningPos &out) {
    // Tokenize. strtok mutates `line`; that's fine since the caller owns it.
    const char *seps = " \t\r\n|[]";
    char *tokens[10] = {0};
    int n = 0;
    char *tok = strtok(line, seps);
    while (tok && n < 10) {
        tokens[n++] = tok;
        tok = strtok(NULL, seps);
    }

    // Need at minimum: 4 FEN fields (placement, stm, castling, ep) + result.
    // hm/fm are optional and don't affect eval.
    if (n < 5) return false;

    // Result is always the last token.
    if (!result_to_float(tokens[n - 1], out.result)) return false;

    strncpy(out.fen,    tokens[0], sizeof(out.fen)    - 1); out.fen[sizeof(out.fen)-1] = '\0';
    strncpy(out.lado,   tokens[1], sizeof(out.lado)   - 1); out.lado[sizeof(out.lado)-1] = '\0';
    strncpy(out.roques, tokens[2], sizeof(out.roques) - 1); out.roques[sizeof(out.roques)-1] = '\0';
    strncpy(out.ep,     tokens[3], sizeof(out.ep)     - 1); out.ep[sizeof(out.ep)-1] = '\0';

    // hm/fm defaulted if not present (eval doesn't use them but setar_posicao expects fields).
    if (n >= 7) {
        strncpy(out.hm, tokens[4], sizeof(out.hm) - 1); out.hm[sizeof(out.hm)-1] = '\0';
        strncpy(out.fm, tokens[5], sizeof(out.fm) - 1); out.fm[sizeof(out.fm)-1] = '\0';
    } else {
        strcpy(out.hm, "0");
        strcpy(out.fm, "1");
    }

    return true;
}

static void load_dataset(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "tuner: cannot open dataset %s: %s\n", path, strerror(errno));
        exit(1);
    }

    char line[512];
    int loaded = 0, skipped = 0;
    while (fgets(line, sizeof(line), f)) {
        // Skip empty lines and comments
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '\n' || *p == '#') continue;

        TuningPos pos;
        if (parse_line(line, pos)) {
            dataset.push_back(pos);
            loaded++;
        } else {
            skipped++;
        }
    }
    fclose(f);

    fprintf(stderr, "tuner: loaded %d positions, skipped %d\n", loaded, skipped);
    if (loaded == 0) {
        fprintf(stderr, "tuner: empty dataset, aborting\n");
        exit(1);
    }
}

static inline double sigmoid(double cp, double K) {
    return 1.0 / (1.0 + std::pow(10.0, -K * cp / 400.0));
}

static double total_loss(double K, const std::vector<TuningPos>& set) {
    double sum = 0.0;
    const int n = (int)set.size();
    for (int i = 0; i < n; i++) {
        TuningPos p = set[i];  // setar_posicao wants non-const char* fields

        Update::setar_posicao(p.fen, p.lado, p.roques, p.ep, p.hm, p.fm);

        int eval_cp = Eval::avaliar();
        if (Game::lado == PRETAS) eval_cp = -eval_cp;

        double pred = sigmoid((double)eval_cp, K);
        double err  = (double)p.result - pred;
        sum += err * err;
    }
    return sum / (double)n;
}

static double optimize_K(const std::vector<TuningPos>& set) {
    double low = 0.1, high = 3.0;
    for (int iter = 0; iter < 30 && (high - low) > 0.0005; iter++) {
        double m1 = low + (high - low) / 3.0;
        double m2 = high - (high - low) / 3.0;
        double l1 = total_loss(m1, set);
        double l2 = total_loss(m2, set);
        if (l1 < l2) high = m2;
        else         low  = m1;
        fprintf(stderr, "tuner: K-opt iter %2d: K in [%.4f, %.4f], loss=%.6f\n",
                iter, low, high, std::min(l1, l2));
    }
    return (low + high) / 2.0;
}

// ---------------------------------------------------------------------------
// Per-parameter sample-support counting.
//
// Plain coordinate descent has no concept of "not enough evidence" — a PST
// cell or mobility bucket that only a handful of training positions ever
// touch gets the exact same ±1 acceptance rule as a cell backed by hundreds
// of thousands of samples, so a few noisy outcomes can drag it to an extreme
// value that minimizes training loss but means nothing (classic overfit
// signature: non-monotonic mobility curves, wild adjacent-square PST swings
// on rarely-occupied squares). Freezing parameters below MIN_SAMPLES removes
// exactly those degrees of freedom instead of letting them fit noise.

static long pst_support[6][CASAS_DO_TABULEIRO];
static long mob_support_c[9], mob_support_b[14], mob_support_t[15], mob_support_d[28];

static void compute_support(const std::vector<TuningPos>& set) {
    memset(pst_support, 0, sizeof(pst_support));
    memset(mob_support_c, 0, sizeof(mob_support_c));
    memset(mob_support_b, 0, sizeof(mob_support_b));
    memset(mob_support_t, 0, sizeof(mob_support_t));
    memset(mob_support_d, 0, sizeof(mob_support_d));

    for (size_t i = 0; i < set.size(); i++) {
        TuningPos p = set[i];  // setar_posicao wants non-const char* fields
        Update::setar_posicao(p.fen, p.lado, p.roques, p.ep, p.hm, p.fm);

        for (int piece = P; piece <= R; piece++) {
            for (int l = 0; l < LADOS; l++) {
                Bitboard::u64 t = Bitboard::bit_pieces[l][piece];
                while (t) {
                    int casa = Bitboard::bitscan(t);
                    t &= Bitboard::not_mask[casa];
                    int x = (l == BRANCAS) ? casa : Consts::flip[casa];
                    pst_support[piece][x]++;
                }
            }
        }

        for (int l = 0; l < LADOS; l++) {
            const Bitboard::u64 nao_proprios = ~Bitboard::bit_lados[l];

            Bitboard::u64 t = Bitboard::bit_pieces[l][C];
            while (t) {
                int casa = Bitboard::bitscan(t); t &= Bitboard::not_mask[casa];
                mob_support_c[Bitboard::popcount(Gen::bit_moves_cavalo[casa] & nao_proprios)]++;
            }
            t = Bitboard::bit_pieces[l][B];
            while (t) {
                int casa = Bitboard::bitscan(t); t &= Bitboard::not_mask[casa];
                mob_support_b[Bitboard::popcount(Gen::atacantes_bispo(casa) & nao_proprios)]++;
            }
            t = Bitboard::bit_pieces[l][T];
            while (t) {
                int casa = Bitboard::bitscan(t); t &= Bitboard::not_mask[casa];
                mob_support_t[Bitboard::popcount(Gen::atacantes_torre(casa) & nao_proprios)]++;
            }
            t = Bitboard::bit_pieces[l][D];
            while (t) {
                int casa = Bitboard::bitscan(t); t &= Bitboard::not_mask[casa];
                Bitboard::u64 att = Gen::atacantes_bispo(casa) | Gen::atacantes_torre(casa);
                mob_support_d[Bitboard::popcount(att & nao_proprios)]++;
            }
        }
    }
}

struct Param {
    Eval::Score* slot;
    Eval::Score* mirror;
    bool is_mg;
    long support;    // training positions where this feature is active
};

static std::vector<Param> params;
static int min_samples = 200;

static void register_params() {
    for (int p = P; p <= R; p++) {
        for (int x = 0; x < CASAS_DO_TABULEIRO; x++) {
            Eval::Score* white = &Eval::score_casas[BRANCAS][p][x];
            Eval::Score* black = &Eval::score_casas[PRETAS][p][Consts::flip[x]];
            long support = pst_support[p][x];
            params.push_back({white, black, true,  support});   // mg
            params.push_back({white, black, false, support});   // eg
        }
    }

    const long DENSE = std::numeric_limits<long>::max();

    for (int i = 0; i < 9;  i++){ params.push_back({&Eval::mobilidade_cavalo[i], NULL, true,  mob_support_c[i]}); params.push_back({&Eval::mobilidade_cavalo[i], NULL, false, mob_support_c[i]}); }
    for (int i = 0; i < 14; i++){ params.push_back({&Eval::mobilidade_bispo[i],  NULL, true,  mob_support_b[i]}); params.push_back({&Eval::mobilidade_bispo[i],  NULL, false, mob_support_b[i]}); }
    for (int i = 0; i < 15; i++){ params.push_back({&Eval::mobilidade_torre[i],  NULL, true,  mob_support_t[i]}); params.push_back({&Eval::mobilidade_torre[i],  NULL, false, mob_support_t[i]}); }
    for (int i = 0; i < 28; i++){ params.push_back({&Eval::mobilidade_dama[i],   NULL, true,  mob_support_d[i]}); params.push_back({&Eval::mobilidade_dama[i],   NULL, false, mob_support_d[i]}); }

    // King-safety weights are dense, always-active scalars (scaled by piece
    // count and king-zone attacks in essentially every position) — gating
    // them doesn't address the sparse-bin overfit problem, so exempt them.
    params.push_back({&Eval::ks_weight_c, NULL, true, DENSE});
    params.push_back({&Eval::ks_weight_b, NULL, true, DENSE});
    params.push_back({&Eval::ks_weight_t, NULL, true, DENSE});
    params.push_back({&Eval::ks_weight_d, NULL, true, DENSE});

    size_t frozen = 0;
    for (size_t i = 0; i < params.size(); i++) if (params[i].support < min_samples) frozen++;

    fprintf(stderr, "tuner: registered %zu parameters (PSTs + mobility + king safety, mg+eg halves)\n", params.size());
    fprintf(stderr, "tuner: %zu/%zu parameters frozen (support < %d training samples)\n",
            frozen, params.size(), min_samples);
}

static inline void tweak(const Param& p, int delta) {
    int mg = Eval::mg_score(*p.slot);
    int eg = Eval::eg_score(*p.slot);
    if (p.is_mg) mg += delta;
    else         eg += delta;
    *p.slot = Eval::make_score(mg, eg);
    if (p.mirror) *p.mirror = *p.slot;
}

// Snapshot/restore the live tunable state. Two Param entries (mg, eg) can
// point at the same packed Score slot, so the snapshot is simply "current
// value per Param index" — restoring writes the same slot twice in that
// case, which is idempotent.
static std::vector<Eval::Score> snapshot_params() {
    std::vector<Eval::Score> snap(params.size());
    for (size_t i = 0; i < params.size(); i++) snap[i] = *params[i].slot;
    return snap;
}

static void restore_params(const std::vector<Eval::Score>& snap) {
    for (size_t i = 0; i < params.size(); i++) {
        *params[i].slot = snap[i];
        if (params[i].mirror) *params[i].mirror = snap[i];
    }
}

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

// Writes `snap` to the checkpoint file without disturbing the live
// optimization state: swap in, print, swap back. Single-threaded, so this
// is safe. Via a temp file + rename so an interrupted write never leaves a
// truncated file.
static void write_checkpoint(int pass, double train_loss, double val_loss,
                              const std::vector<Eval::Score>& snap) {
    if (!checkpoint_path) return;

    std::vector<Eval::Score> live = snapshot_params();
    restore_params(snap);

    char tmp_path[1024];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", checkpoint_path);

    FILE* f = fopen(tmp_path, "w");
    if (!f) {
        fprintf(stderr, "tuner: cannot write checkpoint %s: %s\n", tmp_path, strerror(errno));
        restore_params(live);
        return;
    }
    fprintf(f, "// checkpoint: best validation loss=%.6f (train=%.6f) at pass %d\n",
            val_loss, train_loss, pass);
    print_tuned_values(f);
    fclose(f);

    if (rename(tmp_path, checkpoint_path) != 0) {
        fprintf(stderr, "tuner: cannot rename checkpoint to %s: %s\n", checkpoint_path, strerror(errno));
    }

    restore_params(live);
}

// Coordinate descent, gated by per-parameter sample support and watched by a
// held-out validation set. Training loss alone can't tell overfitting from
// real improvement — it only ever goes down — so acceptance still optimizes
// training loss (that's the actual objective function), but progress is
// judged, checkpointed, and ultimately returned by validation loss. If
// validation loss hasn't improved for `patience` passes, stop early rather
// than continuing to shave training loss at validation's expense.
static double coordinate_descent(double K, int max_passes,
                                  const std::vector<TuningPos>& val_set, int patience) {
    std::mt19937 rng(0xC0FFEE);
    std::vector<size_t> order(params.size());
    for (size_t i = 0; i < params.size(); i++) order[i] = i;

    double best_loss = total_loss(K, dataset);
    double best_val_loss = total_loss(K, val_set);
    std::vector<Eval::Score> best_val_snapshot = snapshot_params();
    int passes_since_val_improved = 0;

    fprintf(stderr, "tuner: starting loss = %.6f (train), %.6f (val)\n", best_loss, best_val_loss);

    for (int pass = 0; max_passes == 0 || pass < max_passes; pass++) {
        std::shuffle(order.begin(), order.end(), rng);

        int accepted = 0, skipped_frozen = 0;
        time_t pass_start = time(NULL);

        for (size_t idx = 0; idx < order.size(); idx++) {
            Param& p = params[order[idx]];

            if (p.support < min_samples) { skipped_frozen++; continue; }

            // Try +1
            tweak(p, +1);
            double loss_plus = total_loss(K, dataset);

            if (loss_plus < best_loss) {
                best_loss = loss_plus;
                accepted++;
                continue;
            }

            // Revert and try -1
            tweak(p, -1);  // undo +1 → back to original
            tweak(p, -1);  // step to -1
            double loss_minus = total_loss(K, dataset);

            if (loss_minus < best_loss) {
                best_loss = loss_minus;
                accepted++;
                continue;
            }

            // Neither direction helped, revert to original.
            tweak(p, +1);
        }

        time_t elapsed = time(NULL) - pass_start;

        double val_loss = total_loss(K, val_set);
        bool improved = val_loss < best_val_loss;
        if (improved) {
            best_val_loss = val_loss;
            best_val_snapshot = snapshot_params();
            passes_since_val_improved = 0;
            write_checkpoint(pass + 1, best_loss, best_val_loss, best_val_snapshot);
        } else {
            passes_since_val_improved++;
        }

        fprintf(stderr, "tuner: pass %d done, accepted %d / %zu (%d frozen), "
                        "loss=%.6f (train), %.6f (val)%s, elapsed=%lds\n",
                pass + 1, accepted, params.size(), skipped_frozen,
                best_loss, val_loss, improved ? " *" : "", (long)elapsed);

        if (accepted == 0) {
            fprintf(stderr, "tuner: converged after pass %d (no improvements)\n", pass + 1);
            break;
        }

        if (patience > 0 && passes_since_val_improved >= patience) {
            fprintf(stderr, "tuner: stopping after pass %d (validation loss hasn't improved in %d passes)\n",
                    pass + 1, patience);
            break;
        }
    }

    fprintf(stderr, "tuner: restoring best-validation snapshot (val loss=%.6f)\n", best_val_loss);
    restore_params(best_val_snapshot);

    return best_val_loss;
}

// ---------------------------------------------------------------------------
// Entry point.

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <dataset.txt> [--max N] [--passes P] [--checkpoint FILE]\n"
                        "                      [--val-frac F] [--min-samples N] [--patience P]\n", argv[0]);
        fprintf(stderr, "  --max N          Cap dataset to N positions (random shuffle then truncate). Default: 200000. 0 = unlimited.\n");
        fprintf(stderr, "  --passes P       Stop after P coord-descent passes. Default: 30. 0 = unlimited.\n");
        fprintf(stderr, "  --checkpoint F   Rewrite F with the best-validation tuned values whenever validation loss improves.\n");
        fprintf(stderr, "  --val-frac F     Fraction of the (post --max) dataset held out for validation. Default: 0.15.\n");
        fprintf(stderr, "  --min-samples N  Freeze any parameter touched by fewer than N training positions. Default: 200.\n");
        fprintf(stderr, "  --patience P     Stop early after P passes with no validation improvement. Default: 20. 0 = disabled.\n");
        return 1;
    }

    int max_positions = 200000;
    int max_passes    = 30;
    double val_frac   = 0.15;
    int patience      = 20;

    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--max") && i + 1 < argc) {
            max_positions = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--passes") && i + 1 < argc) {
            max_passes = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--checkpoint") && i + 1 < argc) {
            checkpoint_path = argv[++i];
        } else if (!strcmp(argv[i], "--val-frac") && i + 1 < argc) {
            val_frac = atof(argv[++i]);
        } else if (!strcmp(argv[i], "--min-samples") && i + 1 < argc) {
            min_samples = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--patience") && i + 1 < argc) {
            patience = atoi(argv[++i]);
        }
    }

    fprintf(stderr, "tuner: initializing engine...\n");
    Init::init();

    fprintf(stderr, "tuner: loading dataset from %s ...\n", argv[1]);
    load_dataset(argv[1]);

    // Random subsample to keep coord-descent passes fast on huge datasets.
    if (max_positions > 0 && (int)dataset.size() > max_positions) {
        std::mt19937 rng(0xDA7A);  // deterministic shuffle for reproducible runs
        std::shuffle(dataset.begin(), dataset.end(), rng);
        dataset.resize(max_positions);
        fprintf(stderr, "tuner: subsampled to %d positions\n", max_positions);
    }

    // Held-out validation split. Coordinate descent never sees these
    // positions as an optimization target — they're the only thing that can
    // tell a real improvement from the optimizer fitting training noise.
    std::vector<TuningPos> val_set;
    if (val_frac > 0.0 && val_frac < 1.0) {
        std::mt19937 rng(0xFEED);
        std::shuffle(dataset.begin(), dataset.end(), rng);
        size_t val_n = (size_t)(dataset.size() * val_frac);
        val_set.assign(dataset.end() - val_n, dataset.end());
        dataset.resize(dataset.size() - val_n);
    }
    fprintf(stderr, "tuner: split into %zu training / %zu validation positions\n",
            dataset.size(), val_set.size());

    fprintf(stderr, "tuner: computing per-parameter sample support...\n");
    compute_support(dataset);

    fprintf(stderr, "tuner: optimizing K ...\n");
    double K = optimize_K(dataset);
    fprintf(stderr, "tuner: optimal K = %.4f\n", K);

    register_params();

    fprintf(stderr, "tuner: starting coordinate descent (max %d passes, patience %d)...\n",
            max_passes, patience);
    double final_val_loss = coordinate_descent(K, max_passes, val_set, patience);
    fprintf(stderr, "tuner: final validation loss = %.6f\n", final_val_loss);

    print_tuned_values(stdout);

    return 0;
}
