/*
 * capizero PV-resolver (Phase B of the self-play data pipeline).
 *
 * For each input line (FEN + WDL result), runs a high-depth search and walks
 * the resulting PV through the TT to its leaf, then emits that leaf with the
 * ORIGINAL game's WDL. Per Ethereal's "Refined Dataset Generation": the leaf
 * is a quiet, deep-search-resolved position whose static eval correlates with
 * the search score; pairing it with the source game's WDL gives a stronger
 * gradient signal for Texel tuning than the unresolved source position.
 *
 * Build with `make resolve`. Run as:
 *   ./capi_resolve --in raw.txt --out resolved.txt [--depth 12] [--tt-mb 16]
 *
 * Input/output format is tuner-compatible: `<FEN> [<result>]` per line.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <ctime>

#include "consts.h"
#include "init.h"
#include "values.h"
#include "game.h"
#include "update.h"
#include "search.h"
#include "hash.h"
#include "interface.h"
#include "gen.h"
#include "bitboard.h"
#include "fen_serializer.h"

// ---------------------------------------------------------------------------
// CLI.

static const char* opt_in       = NULL;
static const char* opt_out      = "selfplay_resolved.txt";
static int         opt_depth    = 12;
static int         opt_tt_mb    = 16;
static int         opt_max_pv   = 20;
static bool        opt_verbose  = false;

static void usage(const char* argv0) {
    fprintf(stderr,
        "Usage: %s --in INPUT [options]\n"
        "  --in FILE       Input file (FEN + result per line) [required]\n"
        "  --out FILE      Output file (default selfplay_resolved.txt)\n"
        "  --depth D       Search depth for PV resolution (default 12)\n"
        "  --max-pv P      Max PV plies to walk (default 20)\n"
        "  --tt-mb M       TT size in MB (default 16)\n"
        "  --verbose       Per-position progress logging\n",
        argv0);
}

static void parse_args(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        const char* a = argv[i];
        if      (!strcmp(a, "--in")     && i+1 < argc) opt_in    = argv[++i];
        else if (!strcmp(a, "--out")    && i+1 < argc) opt_out   = argv[++i];
        else if (!strcmp(a, "--depth")  && i+1 < argc) opt_depth = atoi(argv[++i]);
        else if (!strcmp(a, "--tt-mb")  && i+1 < argc) opt_tt_mb = atoi(argv[++i]);
        else if (!strcmp(a, "--max-pv") && i+1 < argc) opt_max_pv= atoi(argv[++i]);
        else if (!strcmp(a, "--verbose")) opt_verbose = true;
        else if (!strcmp(a, "--help") || !strcmp(a, "-h")) { usage(argv[0]); exit(0); }
        else { fprintf(stderr, "resolve: unknown arg %s\n", a); usage(argv[0]); exit(1); }
    }
    if (!opt_in) { fprintf(stderr, "resolve: --in is required\n"); usage(argv[0]); exit(1); }
}

// ---------------------------------------------------------------------------
// Line parser. Mirrors src/tuner.cpp:parse_line — tokenizes on whitespace plus
// '|', '[', ']' so plain "FEN 1.0", "FEN | 1.0", and "FEN [1.0]" all work.

static bool result_to_float(const char* s, float& out) {
    if (!strcmp(s, "1.0") || !strcmp(s, "1") || !strcmp(s, "1-0"))         { out = 1.0f; return true; }
    if (!strcmp(s, "0.5") || !strcmp(s, "1/2-1/2") || !strcmp(s, "0.5-0.5")){ out = 0.5f; return true; }
    if (!strcmp(s, "0.0") || !strcmp(s, "0") || !strcmp(s, "0-1"))         { out = 0.0f; return true; }
    return false;
}

struct ParsedLine {
    char placement[80];
    char side[2];
    char castle[5];
    char ep[3];
    char hm[6];
    char fm[6];
    float result;
};

static bool parse_line(char* line, ParsedLine& out) {
    const char* seps = " \t\r\n|[]";
    char* tokens[10] = {0};
    int n = 0;
    char* tok = strtok(line, seps);
    while (tok && n < 10) {
        tokens[n++] = tok;
        tok = strtok(NULL, seps);
    }
    if (n < 5) return false;
    if (!result_to_float(tokens[n - 1], out.result)) return false;

    strncpy(out.placement, tokens[0], sizeof(out.placement) - 1); out.placement[sizeof(out.placement)-1] = '\0';
    strncpy(out.side,      tokens[1], sizeof(out.side)      - 1); out.side[sizeof(out.side)-1]           = '\0';
    strncpy(out.castle,    tokens[2], sizeof(out.castle)    - 1); out.castle[sizeof(out.castle)-1]       = '\0';
    strncpy(out.ep,        tokens[3], sizeof(out.ep)        - 1); out.ep[sizeof(out.ep)-1]               = '\0';
    if (n >= 7) {
        strncpy(out.hm, tokens[4], sizeof(out.hm) - 1); out.hm[sizeof(out.hm)-1] = '\0';
        strncpy(out.fm, tokens[5], sizeof(out.fm) - 1); out.fm[sizeof(out.fm)-1] = '\0';
    } else {
        strcpy(out.hm, "0");
        strcpy(out.fm, "1");
    }
    return true;
}

// ---------------------------------------------------------------------------
// Resolve one position: search, walk PV via TT, write leaf with the source
// result. PV walk stops early on: missing TT entry, qsearch-only entry
// (hash_depth < 1), illegal move (broken TT), or max-pv cap.

static void resolve_one(const ParsedLine& p, FILE* out) {
    char placement[80], side[2], castle[5], ep[3], hm[6], fm[6];
    memcpy(placement, p.placement, sizeof(placement));
    memcpy(side,      p.side,      sizeof(side));
    memcpy(castle,    p.castle,    sizeof(castle));
    memcpy(ep,        p.ep,        sizeof(ep));
    memcpy(hm,        p.hm,        sizeof(hm));
    memcpy(fm,        p.fm,        sizeof(fm));

    Update::setar_posicao(placement, side, castle, ep, hm, fm);

    // Fresh TT per position keeps PV walks unambiguous (no stale entries from
    // earlier resolves polluting the bucket). limpar_tt is a single memset of
    // the small (16MB default) TT — cheap.
    Hash::limpar_tt();

    Interface::profundidade_fixa  = true;
    Interface::profundidade_maxima = opt_depth;
    Interface::tempo_fixo          = false;
    Interface::tempo_maximo        = 1 << 25;

    Search::pensar(false);

    // Walk the PV from the TT. Each step: lookup current side's TT, take the
    // recorded best move, apply, advance to the new side. Stop conservatively
    // on any anomaly — under-walking is harmless (we just emit a shallower
    // leaf), over-walking on a corrupted TT entry could write garbage.
    int walked = 0;
    while (walked < opt_max_pv) {
        if (!Hash::hash_lookup(Game::lado)) break;
        if (Hash::hash_depth < 1) break;
        const int from  = Hash::hash_inicio;
        const int to    = Hash::hash_destino;
        const int promo = Hash::hash_promove;
        if (from == to) break;  // null-move sentinel or empty entry
        if (!Update::fazer_lance(from, to, promo)) break;
        walked++;
    }

    char fen[128];
    Fen::serialize(fen);
    fprintf(out, "%s [%.1f]\n", fen, p.result);
}

// ---------------------------------------------------------------------------
// Entry point.

int main(int argc, char** argv) {
    parse_args(argc, argv);

    fprintf(stderr, "resolve: initializing engine...\n");
    Init::init();
    Hash::realocar_tt(opt_tt_mb);

    FILE* in_f  = fopen(opt_in,  "r");
    if (!in_f) {
        fprintf(stderr, "resolve: cannot open %s: %s\n", opt_in, strerror(errno));
        return 1;
    }
    FILE* out_f = fopen(opt_out, "w");
    if (!out_f) {
        fprintf(stderr, "resolve: cannot open %s: %s\n", opt_out, strerror(errno));
        return 1;
    }

    fprintf(stderr, "resolve: %s → %s, depth %d, max-pv %d\n",
            opt_in, opt_out, opt_depth, opt_max_pv);

    const time_t t0 = time(NULL);
    int processed = 0, skipped = 0;
    char line[512];
    while (fgets(line, sizeof(line), in_f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '\n' || *p == '#') continue;

        ParsedLine pl;
        if (!parse_line(line, pl)) { skipped++; continue; }

        resolve_one(pl, out_f);
        processed++;

        if (opt_verbose && processed % 1000 == 0) {
            fprintf(stderr, "resolve: %d processed, %d skipped, %lds elapsed\n",
                    processed, skipped, (long)(time(NULL) - t0));
        }
    }

    fclose(in_f);
    fclose(out_f);
    fprintf(stderr, "resolve: done — %d processed, %d skipped in %lds\n",
            processed, skipped, (long)(time(NULL) - t0));
    return 0;
}
