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

#include "consts.h"
#include "init.h"
#include "game.h"
#include "update.h"
#include "bitboard.h"
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

static std::vector<TuningPos> dataset;

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

static double total_loss(double K) {
    double sum = 0.0;
    const int n = (int)dataset.size();
    for (int i = 0; i < n; i++) {
        TuningPos &p = dataset[i];

        Update::setar_posicao(p.fen, p.lado, p.roques, p.ep, p.hm, p.fm);

        int eval_cp = Eval::avaliar();
        if (Game::lado == PRETAS) eval_cp = -eval_cp;

        double pred = sigmoid((double)eval_cp, K);
        double err  = (double)p.result - pred;
        sum += err * err;
    }
    return sum / (double)n;
}

static double optimize_K() {
    double low = 0.1, high = 3.0;
    for (int iter = 0; iter < 30 && (high - low) > 0.0005; iter++) {
        double m1 = low + (high - low) / 3.0;
        double m2 = high - (high - low) / 3.0;
        double l1 = total_loss(m1);
        double l2 = total_loss(m2);
        if (l1 < l2) high = m2;
        else         low  = m1;
        fprintf(stderr, "tuner: K-opt iter %2d: K in [%.4f, %.4f], loss=%.6f\n",
                iter, low, high, std::min(l1, l2));
    }
    return (low + high) / 2.0;
}


struct Param {
    Eval::Score* slot;
    Eval::Score* mirror;
    bool is_mg;
};

static std::vector<Param> params;

static void register_params() {
    for (int p = P; p <= R; p++) {
        for (int x = 0; x < CASAS_DO_TABULEIRO; x++) {
            Eval::Score* white = &Eval::score_casas[BRANCAS][p][x];
            Eval::Score* black = &Eval::score_casas[PRETAS][p][Consts::flip[x]];
            params.push_back({white, black, true});   // mg
            params.push_back({white, black, false});  // eg
        }
    }
    for (int i = 0; i < 9;  i++){ params.push_back({&Eval::mobilidade_cavalo[i], NULL, true});  params.push_back({&Eval::mobilidade_cavalo[i], NULL, false}); }
    for (int i = 0; i < 14; i++){ params.push_back({&Eval::mobilidade_bispo[i],  NULL, true});  params.push_back({&Eval::mobilidade_bispo[i],  NULL, false}); }
    for (int i = 0; i < 15; i++){ params.push_back({&Eval::mobilidade_torre[i],  NULL, true});  params.push_back({&Eval::mobilidade_torre[i],  NULL, false}); }
    for (int i = 0; i < 28; i++){ params.push_back({&Eval::mobilidade_dama[i],   NULL, true});  params.push_back({&Eval::mobilidade_dama[i],   NULL, false}); }

    params.push_back({&Eval::ks_weight_c, NULL, true});
    params.push_back({&Eval::ks_weight_b, NULL, true});
    params.push_back({&Eval::ks_weight_t, NULL, true});
    params.push_back({&Eval::ks_weight_d, NULL, true});

    fprintf(stderr, "tuner: registered %zu parameters (PSTs + mobility + king safety, mg+eg halves)\n", params.size());
}

static inline void tweak(const Param& p, int delta) {
    int mg = Eval::mg_score(*p.slot);
    int eg = Eval::eg_score(*p.slot);
    if (p.is_mg) mg += delta;
    else         eg += delta;
    *p.slot = Eval::make_score(mg, eg);
    if (p.mirror) *p.mirror = *p.slot;
}


static double coordinate_descent(double K, int max_passes) {
    std::mt19937 rng(0xC0FFEE);
    std::vector<size_t> order(params.size());
    for (size_t i = 0; i < params.size(); i++) order[i] = i;

    double best_loss = total_loss(K);
    fprintf(stderr, "tuner: starting loss = %.6f\n", best_loss);

    for (int pass = 0; max_passes == 0 || pass < max_passes; pass++) {
        std::shuffle(order.begin(), order.end(), rng);

        int accepted = 0;
        time_t pass_start = time(NULL);

        for (size_t idx = 0; idx < order.size(); idx++) {
            Param& p = params[order[idx]];

            // Try +1
            tweak(p, +1);
            double loss_plus = total_loss(K);

            if (loss_plus < best_loss) {
                best_loss = loss_plus;
                accepted++;
                continue;
            }

            // Revert and try -1
            tweak(p, -1);  // undo +1 → back to original
            tweak(p, -1);  // step to -1
            double loss_minus = total_loss(K);

            if (loss_minus < best_loss) {
                best_loss = loss_minus;
                accepted++;
                continue;
            }

            // Neither direction helped, revert to original.
            tweak(p, +1);
        }

        time_t elapsed = time(NULL) - pass_start;
        fprintf(stderr, "tuner: pass %d done, accepted %d / %zu, loss=%.6f, elapsed=%lds\n",
                pass + 1, accepted, params.size(), best_loss, (long)elapsed);

        if (accepted == 0) {
            fprintf(stderr, "tuner: converged after pass %d (no improvements)\n", pass + 1);
            break;
        }
    }

    return best_loss;
}

static void print_pst_array(const char* name, int piece, bool is_mg) {
    fprintf(stdout, "\tconst int %s[64] = {\n", name);
    for (int rank = 0; rank < 8; rank++) {
        fprintf(stdout, "\t\t");
        for (int file = 0; file < 8; file++) {
            int x = rank * 8 + file;
            int v = is_mg ? Eval::mg_score(Eval::score_casas[BRANCAS][piece][x])
                          : Eval::eg_score(Eval::score_casas[BRANCAS][piece][x]);
            fprintf(stdout, "%5d%s", v, (file == 7 && rank == 7) ? "\n" : ",");
            if (file == 7 && rank != 7) fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, "\t};\n\n");
}

static void print_mobility_array(const char* name, const Eval::Score* tbl, int len, bool is_mg) {
    fprintf(stdout, "\tconst int %s[%d] = {\n\t\t", name, len);
    for (int i = 0; i < len; i++) {
        int v = is_mg ? Eval::mg_score(tbl[i]) : Eval::eg_score(tbl[i]);
        fprintf(stdout, "%5d%s", v, (i == len - 1) ? "\n" : ",");
        if ((i + 1) % 8 == 0 && i != len - 1) fprintf(stdout, "\n\t\t");
    }
    fprintf(stdout, "\t};\n\n");
}

static void print_tuned_values() {
    fprintf(stdout, "// ==== TUNED VALUES (paste into values.h, set VALOR_*_MG/_EG to 0) ====\n");
    fprintf(stdout, "// PSTs below have material baked in.\n\n");

    print_pst_array("peao_score_mg",   P, true);
    print_pst_array("peao_score_eg",   P, false);
    print_pst_array("cavalo_score_mg", C, true);
    print_pst_array("cavalo_score_eg", C, false);
    print_pst_array("bispo_score_mg",  B, true);
    print_pst_array("bispo_score_eg",  B, false);
    print_pst_array("torre_score_mg",  T, true);
    print_pst_array("torre_score_eg",  T, false);
    print_pst_array("dama_score_mg",   D, true);
    print_pst_array("dama_score_eg",   D, false);
    print_pst_array("rei_score_mg",    R, true);
    print_pst_array("rei_score_eg",    R, false);

    print_mobility_array("mobilidade_cavalo_mg", Eval::mobilidade_cavalo, 9,  true);
    print_mobility_array("mobilidade_cavalo_eg", Eval::mobilidade_cavalo, 9,  false);
    print_mobility_array("mobilidade_bispo_mg",  Eval::mobilidade_bispo,  14, true);
    print_mobility_array("mobilidade_bispo_eg",  Eval::mobilidade_bispo,  14, false);
    print_mobility_array("mobilidade_torre_mg",  Eval::mobilidade_torre,  15, true);
    print_mobility_array("mobilidade_torre_eg",  Eval::mobilidade_torre,  15, false);
    print_mobility_array("mobilidade_dama_mg",   Eval::mobilidade_dama,   28, true);
    print_mobility_array("mobilidade_dama_eg",   Eval::mobilidade_dama,   28, false);

    fprintf(stdout, "\t#define KS_WEIGHT_C %d\n",   Eval::mg_score(Eval::ks_weight_c));
    fprintf(stdout, "\t#define KS_WEIGHT_B %d\n",   Eval::mg_score(Eval::ks_weight_b));
    fprintf(stdout, "\t#define KS_WEIGHT_T %d\n",   Eval::mg_score(Eval::ks_weight_t));
    fprintf(stdout, "\t#define KS_WEIGHT_D %d\n\n", Eval::mg_score(Eval::ks_weight_d));

    fprintf(stdout, "// ==== end tuned values ====\n");
}

// ---------------------------------------------------------------------------
// Entry point.

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <dataset.txt> [--max N] [--passes P]\n", argv[0]);
        fprintf(stderr, "  --max N      Cap dataset to N positions (random shuffle then truncate). Default: 200000. 0 = unlimited.\n");
        fprintf(stderr, "  --passes P   Stop after P coord-descent passes. Default: 30. 0 = unlimited.\n");
        return 1;
    }

    int max_positions = 200000;
    int max_passes    = 30;

    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--max") && i + 1 < argc) {
            max_positions = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--passes") && i + 1 < argc) {
            max_passes = atoi(argv[++i]);
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

    fprintf(stderr, "tuner: optimizing K ...\n");
    double K = optimize_K();
    fprintf(stderr, "tuner: optimal K = %.4f\n", K);

    register_params();

    fprintf(stderr, "tuner: starting coordinate descent (max %d passes)...\n", max_passes);
    double final_loss = coordinate_descent(K, max_passes);
    fprintf(stderr, "tuner: final loss = %.6f\n", final_loss);

    print_tuned_values();

    return 0;
}
