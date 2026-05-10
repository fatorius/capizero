/*
 * Texel-style eval tuner for capizero.
 *
 * Commit 1 (this file): scaffolding + FEN parser + dataset loader + loss
 * function + K (sigmoid-scaling) optimization. Reports the initial loss for
 * the engine's current eval parameters. Does NOT yet mutate parameters.
 *
 * Commit 2 (follow-up): coordinate-descent tuning loop that walks every
 * Eval::score_casas slot and Eval::mobilidade_* entry, accepting any
 * ±1 step that reduces total dataset loss. Output formatting for new
 * values.h content.
 *
 * Build with `make tuner`. Run as: `./capi_tuner <dataset.txt>`.
 *
 * Dataset format (one position per line; tokenizer splits on whitespace,
 * '|', and brackets so common variants all work):
 *
 *   <FEN piece placement> <stm> <castling> <ep> <hm> <fm> <result>
 *
 * where <result> is one of: 1.0 / 0.5 / 0.0 (or 1-0 / 1/2-1/2 / 0-1).
 *
 * Examples:
 *   rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 [0.5]
 *   r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4 1.0
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "consts.h"
#include "init.h"
#include "game.h"
#include "update.h"
#include "bitboard.h"
#include "eval.h"

// Compact per-position record. Stored as parsed FEN tokens so the inner loop
// can re-set the board cheaply via Update::setar_posicao without re-tokenizing
// strings every pass.
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

// ---------------------------------------------------------------------------
// FEN line parsing.
// Accepts tokens separated by space, tab, '|', '[', or ']' to handle the
// common dataset variants (Stockfish-style "FEN [1.0]", Ethereal-style
// "FEN | 1.0", or plain "FEN 1.0"). Missing hm/fm fields default to 0/1.

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

// ---------------------------------------------------------------------------
// Loss function. For each position, set up the board, get the static eval
// from white's perspective, apply sigmoid, compute squared error vs game
// result. Total loss = mean over dataset.
//
// `K` is the sigmoid scaling constant; optimal K varies per engine (different
// eval scales) and is the first thing tuned. Standard formulation:
//   P(white wins) = 1 / (1 + 10^(-K * eval_cp / 400))

static inline double sigmoid(double cp, double K) {
    return 1.0 / (1.0 + std::pow(10.0, -K * cp / 400.0));
}

static double total_loss(double K) {
    double sum = 0.0;
    const int n = (int)dataset.size();
    for (int i = 0; i < n; i++) {
        TuningPos &p = dataset[i];

        // setar_posicao takes non-const char* (legacy C-style API). Cast
        // away const for the call; the function doesn't actually mutate
        // the input strings.
        Update::setar_posicao(p.fen, p.lado, p.roques, p.ep, p.hm, p.fm);

        // Eval::avaliar returns the score from the side-to-move's perspective.
        // Normalize to white's perspective for the sigmoid: a positive number
        // always means "good for white".
        int eval_cp = Eval::avaliar();
        if (Game::lado == PRETAS) eval_cp = -eval_cp;

        double pred = sigmoid((double)eval_cp, K);
        double err  = (double)p.result - pred;
        sum += err * err;
    }
    return sum / (double)n;
}

// ---------------------------------------------------------------------------
// K optimization via ternary search.
// The loss function is convex in K (single minimum), so ternary search
// converges to the optimum in O(log) iterations. ~30 iterations gives
// K precision around 1e-4 starting from [0.1, 3.0].

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

// ---------------------------------------------------------------------------
// Entry point. Initialize engine, load dataset, find best K, report loss.
// Coordinate descent ships in a follow-up commit.

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <dataset.txt>\n", argv[0]);
        fprintf(stderr, "  Dataset format: one position per line, FEN fields + result.\n");
        fprintf(stderr, "  Result token: 1.0 / 0.5 / 0.0 (or 1-0 / 1/2-1/2 / 0-1).\n");
        return 1;
    }

    fprintf(stderr, "tuner: initializing engine...\n");
    Init::init();

    fprintf(stderr, "tuner: loading dataset from %s ...\n", argv[1]);
    load_dataset(argv[1]);

    fprintf(stderr, "tuner: optimizing K ...\n");
    double K = optimize_K();
    double loss = total_loss(K);

    fprintf(stderr, "tuner: optimal K = %.4f, initial loss = %.6f\n", K, loss);
    fprintf(stderr, "tuner: skeleton ready. Coordinate descent ships in commit 2.\n");

    return 0;
}
