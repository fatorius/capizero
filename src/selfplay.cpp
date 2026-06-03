/*
 * capizero self-play data collector (Phase A).
 *
 * Plays engine-vs-engine games with fixed-time or fixed-depth search per move,
 * filters out non-quiet positions (in-check, opening plies, capture/check best
 * moves, eval-vs-qsearch divergence), and dumps each surviving position with
 * the game's eventual WDL result as a tuner-compatible line:
 *
 *     <FEN> [1.0|0.5|0.0]
 *
 * The output feeds capi_resolve (Phase B), which walks each position's PV to a
 * leaf, producing the final dataset for capi_tuner.
 *
 * Build with `make selfplay`. Run as: `./capi_selfplay --games N [opts]`.
 * See parse_args() / usage() for the full flag list.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cerrno>
#include <ctime>
#include <cmath>
#include <string>
#include <vector>
#include <random>
#include <algorithm>

#include "consts.h"
#include "init.h"
#include "values.h"
#include "game.h"
#include "update.h"
#include "search.h"
#include "hash.h"
#include "interface.h"
#include "gen.h"
#include "attacks.h"
#include "bitboard.h"
#include "eval.h"
#include "fen_serializer.h"

// Static qsearch entry lives in search.cpp without a header declaration; expose
// via extern so the eval-vs-qsearch quiet filter can call it.
extern int pesquisa_rapida(int alpha, int beta);

// ---------------------------------------------------------------------------
// CLI options.

static int   opt_games            = 1;
static int   opt_depth            = 0;        // 0 = use time
static int   opt_time_ms          = 100;
static int   opt_max_plies        = 400;      // hard cap to bail out of stalls
static int   opt_skip_open_plies  = 10;       // don't dump positions until hply >= this
static int   opt_max_pos_per_game = 12;       // sample cap; random subset if exceeded
static int   opt_eval_qs_diff_cp  = 50;       // tactical-filter threshold
static int   opt_resign_plies     = 4;        // |score| > resign_cp for K plies → adjudicate
static int   opt_resign_cp        = 800;
static int   opt_draw_plies       = 8;
static int   opt_draw_cp          = 8;
static int   opt_tt_mb            = 64;
static int   opt_random_open_plies= 4;        // random plies on top of startpos (or book pos)
static const char* opt_out        = "selfplay_raw.txt";
static const char* opt_book       = NULL;
static unsigned int opt_seed      = 0;
static bool  opt_smoke            = false;
static bool  opt_verbose          = false;

static void usage(const char* argv0) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "  --games N                Number of games to play (default 1)\n"
        "  --out FILE               Output file (default selfplay_raw.txt)\n"
        "  --time-per-move MS       Fixed time per move in ms (default 100)\n"
        "  --depth-fixed D          Override: fixed depth instead of time\n"
        "  --opening-plies P        Random plies after book setup (default 4)\n"
        "  --book FILE              Opening book EPD (one position per line)\n"
        "  --skip-opening-plies N   Skip dumping positions until hply >= N (default 10)\n"
        "  --resign-plies K         Adjudicate winner if |score|>resign-cp for K plies (default 4)\n"
        "  --resign-cp CP           Adjudication threshold in cp (default 800)\n"
        "  --draw-plies K           Adjudicate draw if endgame stalls (default 8)\n"
        "  --draw-cp CP             Draw-score threshold in cp (default 8)\n"
        "  --max-plies N            Hard game-length cap (default 400)\n"
        "  --max-positions-per-game M  Sample cap per game (default 12; random subset)\n"
        "  --eval-qs-diff CP        Tactical filter: skip if |eval-qsearch|>CP (default 50)\n"
        "  --tt-mb M                TT size in MB (default 16)\n"
        "  --seed S                 RNG seed (default = time())\n"
        "  --verbose                Print per-game progress\n"
        "  --smoke                  Run FEN round-trip smoke test and exit\n",
        argv0);
}

static void parse_args(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        const char* a = argv[i];
        #define ARG_INT(name, var)  else if (!strcmp(a, name) && i+1 < argc) var = atoi(argv[++i])
        #define ARG_STR(name, var)  else if (!strcmp(a, name) && i+1 < argc) var = argv[++i]
        if (!strcmp(a, "--help") || !strcmp(a, "-h")) { usage(argv[0]); exit(0); }
        ARG_INT("--games",                    opt_games);
        ARG_INT("--depth-fixed",              opt_depth);
        ARG_INT("--time-per-move",            opt_time_ms);
        ARG_STR("--out",                      opt_out);
        ARG_STR("--book",                     opt_book);
        ARG_INT("--opening-plies",            opt_random_open_plies);
        ARG_INT("--skip-opening-plies",       opt_skip_open_plies);
        ARG_INT("--max-plies",                opt_max_plies);
        ARG_INT("--max-positions-per-game",   opt_max_pos_per_game);
        ARG_INT("--eval-qs-diff",             opt_eval_qs_diff_cp);
        ARG_INT("--resign-plies",             opt_resign_plies);
        ARG_INT("--resign-cp",                opt_resign_cp);
        ARG_INT("--draw-plies",               opt_draw_plies);
        ARG_INT("--draw-cp",                  opt_draw_cp);
        ARG_INT("--tt-mb",                    opt_tt_mb);
        else if (!strcmp(a, "--seed") && i+1 < argc) opt_seed = (unsigned)strtoul(argv[++i], NULL, 10);
        else if (!strcmp(a, "--smoke"))   opt_smoke = true;
        else if (!strcmp(a, "--verbose")) opt_verbose = true;
        else {
            fprintf(stderr, "selfplay: unknown arg %s\n", a);
            usage(argv[0]);
            exit(1);
        }
        #undef ARG_INT
        #undef ARG_STR
    }
    if (opt_seed == 0) opt_seed = (unsigned)time(NULL);
}

// ---------------------------------------------------------------------------
// FEN round-trip smoke test. Sets the board via Update::setar_posicao on a
// handful of FENs covering common edge cases (startpos, castling, EP, mid-
// game), serializes back, and checks for string equality. Run with --smoke.

static int smoke_test() {
    // setar_posicao discards FEN fullmove (always resets hply=1), so we can't
    // round-trip the fullmove field on non-startpos FENs. The smoke test
    // works at the engine-state level: after setar_posicao, Fen::serialize
    // should emit a FEN whose first FIVE fields exactly match the input, and
    // whose sixth field (fullmove) matches the engine's reset hply=1.
    struct Case {
        const char* placement;
        const char* side;
        const char* castle;
        const char* ep;
        const char* hm;
        const char* fm;
        const char* expected;   // expected serialization output
    };

    Case cases[] = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", "w", "KQkq", "-", "0", "1",
         "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
        // Mid-game with mixed castling and side-to-move black
        {"r3k2r/pp3ppp/2n1b3/8/8/2N1B3/PP3PPP/R3K2R", "b", "KQkq", "-", "0", "12",
         "r3k2r/pp3ppp/2n1b3/8/8/2N1B3/PP3PPP/R3K2R b KQkq - 0 1"},
        // No castling rights, advanced halfmove counter
        {"8/3kp3/1p3bp1/8/1B1P1PK1/4P3/8/8", "b", "-", "-", "0", "40",
         "8/3kp3/1p3bp1/8/1B1P1PK1/4P3/8/8 b - - 0 1"},
        // Asymmetric castling rights
        {"r3k2r/8/8/8/8/8/8/R3K2R", "w", "Kq", "-", "5", "20",
         "r3k2r/8/8/8/8/8/8/R3K2R w Kq - 5 1"},
    };
    const int N = (int)(sizeof(cases) / sizeof(cases[0]));

    int failed = 0;
    char buf[128];
    for (int i = 0; i < N; i++) {
        Case& c = cases[i];
        // setar_posicao takes non-const char* (legacy C-style); copy to mutable
        // buffers per call.
        char placement[80], side[2], castle[5], ep[3], hm[6], fm[6];
        strncpy(placement, c.placement, 79); placement[79] = '\0';
        strncpy(side,      c.side,      1);  side[1]  = '\0';
        strncpy(castle,    c.castle,    4);  castle[4]= '\0';
        strncpy(ep,        c.ep,        2);  ep[2]    = '\0';
        strncpy(hm,        c.hm,        5);  hm[5]    = '\0';
        strncpy(fm,        c.fm,        5);  fm[5]    = '\0';
        Update::setar_posicao(placement, side, castle, ep, hm, fm);

        Fen::serialize(buf);

        if (strcmp(buf, c.expected) != 0) {
            fprintf(stderr, "smoke FAIL [%d]:\n  expected: %s\n  got:      %s\n",
                    i, c.expected, buf);
            failed++;
        } else {
            fprintf(stderr, "smoke OK   [%d]: %s\n", i, buf);
        }
    }

    // EP-detection test: set up a position, apply a 2-square pawn push, and
    // check the EP field of the serialized output.
    {
        char placement[80] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
        char side[2] = "w", castle[5] = "KQkq", ep[3] = "-", hm[6] = "0", fm[6] = "1";
        Update::setar_posicao(placement, side, castle, ep, hm, fm);
        // e2-e4 (sq 12 → sq 28)
        if (!Update::fazer_lance(12, 28)) {
            fprintf(stderr, "smoke FAIL: e2e4 illegal?\n");
            failed++;
        } else {
            Fen::serialize(buf);
            const char* want = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
            if (strcmp(buf, want) != 0) {
                fprintf(stderr, "smoke FAIL EP after e2e4:\n  expected: %s\n  got:      %s\n", want, buf);
                failed++;
            } else {
                fprintf(stderr, "smoke OK   EP after e2e4: %s\n", buf);
            }
        }
    }

    fprintf(stderr, "smoke: %d failures\n", failed);
    return failed;
}

// ---------------------------------------------------------------------------
// Opening book loader. EPD format: one position per line, first 4-6 whitespace-
// separated tokens parsed as FEN fields (placement, side, castle, ep, [hm, fm]).
// Trailing EPD operations (bm, id, c0, etc.) are ignored.

struct BookEntry {
    char placement[80];
    char side[2];
    char castle[5];
    char ep[3];
    char hm[6];
    char fm[6];
};

static std::vector<BookEntry> book;

static void load_book(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "selfplay: cannot open book %s\n", path);
        exit(1);
    }
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *tok = strtok(line, " \t\r\n");
        if (!tok || *tok == '#') continue;
        BookEntry e;
        strncpy(e.placement, tok, sizeof(e.placement) - 1); e.placement[sizeof(e.placement)-1] = '\0';

        tok = strtok(NULL, " \t\r\n");
        if (!tok) continue;
        strncpy(e.side, tok, sizeof(e.side) - 1); e.side[sizeof(e.side)-1] = '\0';

        tok = strtok(NULL, " \t\r\n");
        if (!tok) continue;
        strncpy(e.castle, tok, sizeof(e.castle) - 1); e.castle[sizeof(e.castle)-1] = '\0';

        tok = strtok(NULL, " \t\r\n");
        if (!tok) continue;
        strncpy(e.ep, tok, sizeof(e.ep) - 1); e.ep[sizeof(e.ep)-1] = '\0';

        tok = strtok(NULL, " \t\r\n");
        strncpy(e.hm, tok ? tok : "0", sizeof(e.hm) - 1); e.hm[sizeof(e.hm)-1] = '\0';

        tok = strtok(NULL, " \t\r\n");
        strncpy(e.fm, tok ? tok : "1", sizeof(e.fm) - 1); e.fm[sizeof(e.fm)-1] = '\0';

        book.push_back(e);
    }
    fclose(f);
    fprintf(stderr, "selfplay: loaded %zu book positions from %s\n", book.size(), path);
}

// ---------------------------------------------------------------------------
// Legal-move enumeration helpers (used for termination + random opening plies).

// Counts legal moves for the current side. Side effects: regenerates
// Gen::lista_de_lances at Game::ply (caller must ensure ply is sane).
static int count_legal_moves() {
    Gen::gerar_lances(Game::lado, Game::xlado);
    int count = 0;
    for (int i = Game::qntt_lances_totais[Game::ply]; i < Game::qntt_lances_totais[Game::ply + 1]; i++) {
        const Gen::lance& m = Gen::lista_de_lances[i];
        if (Update::fazer_lance(m.inicio, m.destino, m.promove)) {
            Update::desfaz_lance();
            count++;
        }
    }
    return count;
}

// Picks and applies a random legal move. Returns false if no legal moves.
static bool apply_random_legal_move(std::mt19937& rng) {
    Gen::gerar_lances(Game::lado, Game::xlado);
    std::vector<int> legal_indices;
    for (int i = Game::qntt_lances_totais[Game::ply]; i < Game::qntt_lances_totais[Game::ply + 1]; i++) {
        const Gen::lance& m = Gen::lista_de_lances[i];
        if (Update::fazer_lance(m.inicio, m.destino, m.promove)) {
            Update::desfaz_lance();
            legal_indices.push_back(i);
        }
    }
    if (legal_indices.empty()) return false;
    const int pick = legal_indices[rng() % legal_indices.size()];
    const Gen::lance& m = Gen::lista_de_lances[pick];
    return Update::fazer_lance(m.inicio, m.destino, m.promove);
}

// ---------------------------------------------------------------------------
// Game-state helpers.

static bool side_to_move_in_check() {
    const int king_sq = Bitboard::bitscan(Bitboard::bit_pieces[Game::lado][R]);
    return Attacks::casa_esta_sendo_atacada(Game::xlado, king_sq);
}

// Insufficient material: both sides have 0 pawns + 0 rooks + 0 queens, and
// at most one minor piece each.
static bool insufficient_material() {
    if (Bitboard::bit_pieces[BRANCAS][P] | Bitboard::bit_pieces[PRETAS][P]) return false;
    if (Bitboard::bit_pieces[BRANCAS][T] | Bitboard::bit_pieces[PRETAS][T]) return false;
    if (Bitboard::bit_pieces[BRANCAS][D] | Bitboard::bit_pieces[PRETAS][D]) return false;
    const int white_minors = Bitboard::popcount(Bitboard::bit_pieces[BRANCAS][C])
                           + Bitboard::popcount(Bitboard::bit_pieces[BRANCAS][B]);
    const int black_minors = Bitboard::popcount(Bitboard::bit_pieces[PRETAS][C])
                           + Bitboard::popcount(Bitboard::bit_pieces[PRETAS][B]);
    return white_minors <= 1 && black_minors <= 1;
}

// Convert an engine score (from side-to-move's perspective) to white's
// perspective. We always log results from white's POV.
static int score_white_pov(int score, int side_to_move) {
    return (side_to_move == BRANCAS) ? score : -score;
}

// ---------------------------------------------------------------------------
// Per-game state reset.

static void reset_for_new_game() {
    Game::novo_jogo();
    Hash::limpar_tt();
    memset(Search::historico_heuristica,  0, sizeof(Search::historico_heuristica));
    memset(Search::contraLance_heuristica,0, sizeof(Search::contraLance_heuristica));
    memset(Search::killers_primarios,     0, sizeof(Search::killers_primarios));
    memset(Search::killers_secundarios,   0, sizeof(Search::killers_secundarios));
}

static void configure_search() {
    if (opt_depth > 0) {
        Interface::profundidade_fixa  = true;
        Interface::profundidade_maxima = opt_depth;
        Interface::tempo_fixo          = false;
        Interface::tempo_maximo        = 1 << 25;
    } else {
        Interface::profundidade_fixa   = false;
        Interface::profundidade_maxima = MAX_PLY - 2;
        Interface::tempo_fixo          = true;
        Interface::tempo_maximo        = opt_time_ms;
    }
}

// ---------------------------------------------------------------------------
// Game termination decision after a move was applied.
//
// Returns true if game is over and sets `result` to {1.0, 0.5, 0.0} from
// white's perspective. Adjudication uses recent-score history.

struct ScoreHistory {
    int recent[16];  // ring buffer of recent root scores (white POV)
    int count;
    void clear()       { count = 0; }
    void push(int cp)  { recent[count % 16] = cp; count++; }
    bool win_streak(int plies, int cp_threshold, int& winner_side) const {
        if (count < plies) return false;
        bool all_white = true, all_black = true;
        for (int k = 0; k < plies; k++) {
            const int s = recent[(count - 1 - k) % 16];
            if (s <  cp_threshold) all_white = false;
            if (s > -cp_threshold) all_black = false;
        }
        if (all_white) { winner_side = BRANCAS; return true; }
        if (all_black) { winner_side = PRETAS;  return true; }
        return false;
    }
    bool draw_streak(int plies, int cp_threshold) const {
        if (count < plies) return false;
        for (int k = 0; k < plies; k++) {
            const int s = recent[(count - 1 - k) % 16];
            if (std::abs(s) > cp_threshold) return false;
        }
        return true;
    }
};

// ---------------------------------------------------------------------------
// Main game loop.

static int play_one_game(FILE* out, std::mt19937& rng) {
    reset_for_new_game();

    // Opening setup
    if (!book.empty()) {
        const BookEntry& e = book[rng() % book.size()];
        char placement[80], side[2], castle[5], ep[3], hm[6], fm[6];
        strncpy(placement, e.placement, 79); placement[79] = '\0';
        strncpy(side,      e.side,      1);  side[1]  = '\0';
        strncpy(castle,    e.castle,    4);  castle[4]= '\0';
        strncpy(ep,        e.ep,        2);  ep[2]    = '\0';
        strncpy(hm,        e.hm,        5);  hm[5]    = '\0';
        strncpy(fm,        e.fm,        5);  fm[5]    = '\0';
        Update::setar_posicao(placement, side, castle, ep, hm, fm);
    }
    for (int p = 0; p < opt_random_open_plies; p++) {
        if (!apply_random_legal_move(rng)) break;
    }

    configure_search();

    std::vector<std::string> per_game_buffer;
    per_game_buffer.reserve(64);

    ScoreHistory sh;
    sh.clear();

    float result = 0.5f;
    bool decided = false;

    while (Game::hply < opt_max_plies) {
        // Search this position.
        Search::pensar(false);

        // Read root TT entry: holds best move + score from side-to-move POV.
        // The entry can be missing if the search hit timeout after a colliding
        // child entry evicted the root's previous-iteration entry — that's a
        // real failure mode under tight TC + small TT, not a mate. So we first
        // disambiguate by counting legal moves; if any exist, we recover by
        // picking the first legal move and continuing the game (suboptimal
        // play this turn, but the dataset doesn't depend on optimality —
        // just on getting a labeled WDL out of the game).
        const int side_at_root = Game::lado;
        int from, to, promo, score_stm;
        if (!Hash::hash_lookup(side_at_root)) {
            Game::ply = 0;
            const int legal = count_legal_moves();
            if (legal == 0) {
                if (side_to_move_in_check()) {
                    result = (side_at_root == BRANCAS) ? 0.0f : 1.0f;
                } else {
                    result = 0.5f;
                }
                decided = true;
                break;
            }
            // Fallback: first legal move from gerar_lances.
            Gen::gerar_lances(Game::lado, Game::xlado);
            const Gen::lance* picked = NULL;
            for (int i = Game::qntt_lances_totais[Game::ply]; i < Game::qntt_lances_totais[Game::ply + 1]; i++) {
                const Gen::lance& m = Gen::lista_de_lances[i];
                if (Update::fazer_lance(m.inicio, m.destino, m.promove)) {
                    Update::desfaz_lance();
                    picked = &Gen::lista_de_lances[i];
                    break;
                }
            }
            if (!picked) {
                // Shouldn't reach — legal > 0 above. Bail safely.
                result = 0.5f;
                decided = true;
                break;
            }
            from = picked->inicio; to = picked->destino; promo = picked->promove;
            score_stm = 0;
        } else {
            from      = Hash::hash_inicio;
            to        = Hash::hash_destino;
            promo     = Hash::hash_promove;
            score_stm = Hash::hash_score;
        }

        // Quiet-position filters BEFORE applying the move.
        bool dump_this = (Game::hply >= opt_skip_open_plies);
        if (dump_this) {
            if (side_to_move_in_check()) dump_this = false;
        }
        if (dump_this) {
            if (Bitboard::tabuleiro[to] != VAZIO) dump_this = false;  // capture
            if (promo != 0)                       dump_this = false;  // promotion
        }
        if (dump_this) {
            // Static eval vs qsearch divergence (tactical positions).
            const int eval_static = Eval::avaliar();
            const int eval_qs     = pesquisa_rapida(ALPHA_INICIAL, BETA_INICIAL);
            if (std::abs(eval_static - eval_qs) > opt_eval_qs_diff_cp) dump_this = false;
        }

        char fen_buf[128];
        if (dump_this) {
            Fen::serialize(fen_buf);
        }

        // Apply the bestmove.
        if (!Update::fazer_lance(from, to, promo)) {
            fprintf(stderr, "selfplay: illegal bestmove (hply=%d from=%d to=%d promo=%d)\n",
                    Game::hply, from, to, promo);
            break;
        }

        // Post-move filter: did the move give check? If so, the pre-move
        // position was tactical (forcing) — skip.
        if (dump_this) {
            const int now_king_sq = Bitboard::bitscan(Bitboard::bit_pieces[Game::lado][R]);
            if (Attacks::casa_esta_sendo_atacada(Game::xlado, now_king_sq)) {
                dump_this = false;
            }
        }
        if (dump_this) {
            per_game_buffer.emplace_back(fen_buf);
        }

        // Record score for adjudication (always white POV).
        sh.push(score_white_pov(score_stm, side_at_root));

        // Termination: rules first.
        if (Game::cinquenta >= 100)    { result = 0.5f; decided = true; break; }
        if (Game::checar_repeticoes()) { result = 0.5f; decided = true; break; }
        if (insufficient_material())   { result = 0.5f; decided = true; break; }

        // Termination: mate/stalemate via legal-move count for the new side.
        Game::ply = 0;
        const int legal = count_legal_moves();
        if (legal == 0) {
            if (side_to_move_in_check()) {
                result = (Game::lado == BRANCAS) ? 0.0f : 1.0f;
            } else {
                result = 0.5f;
            }
            decided = true;
            break;
        }

        // Termination: adjudication.
        int winner_side;
        if (sh.win_streak(opt_resign_plies, opt_resign_cp, winner_side)) {
            result = (winner_side == BRANCAS) ? 1.0f : 0.0f;
            decided = true;
            break;
        }
        if (sh.draw_streak(opt_draw_plies, opt_draw_cp)) {
            result = 0.5f;
            decided = true;
            break;
        }
    }

    if (!decided) {
        // Hit max-plies cap without a decisive result — call it a draw.
        result = 0.5f;
    }

    // Random-subsample if we collected more than max_pos_per_game.
    if ((int)per_game_buffer.size() > opt_max_pos_per_game) {
        std::shuffle(per_game_buffer.begin(), per_game_buffer.end(), rng);
        per_game_buffer.resize(opt_max_pos_per_game);
    }

    for (const auto& fen : per_game_buffer) {
        fprintf(out, "%s [%.1f]\n", fen.c_str(), result);
    }

    return (int)per_game_buffer.size();
}

// ---------------------------------------------------------------------------
// Entry point.

int main(int argc, char** argv) {
    parse_args(argc, argv);

    fprintf(stderr, "selfplay: initializing engine...\n");
    Init::init();

    if (opt_smoke) {
        return smoke_test();
    }

    Hash::realocar_tt(opt_tt_mb);

    if (opt_book) {
        load_book(opt_book);
    }

    FILE* out = fopen(opt_out, "w");
    if (!out) {
        fprintf(stderr, "selfplay: cannot open output %s: %s\n", opt_out, strerror(errno));
        return 1;
    }

    std::mt19937 rng(opt_seed);

    fprintf(stderr, "selfplay: playing %d games (TC: %s) → %s, seed=%u\n",
            opt_games,
            opt_depth > 0 ? "fixed-depth" : "fixed-time",
            opt_out, opt_seed);

    const time_t t0 = time(NULL);
    int total_positions = 0;
    for (int g = 1; g <= opt_games; g++) {
        const int n = play_one_game(out, rng);
        total_positions += n;
        if (opt_verbose || g % 100 == 0) {
            const long elapsed = (long)(time(NULL) - t0);
            const double gps   = elapsed > 0 ? (double)g / elapsed : 0.0;
            const long   eta   = gps > 0 ? (long)((opt_games - g) / gps) : 0;
            fprintf(stderr,
                "selfplay: game %d/%d | positions %d | %.1f games/s | elapsed %lds | ETA %lds\n",
                g, opt_games, total_positions, gps, elapsed, eta);
            fflush(out);
        }
    }

    fclose(out);
    fprintf(stderr, "selfplay: done — %d games, %d positions in %lds\n",
            opt_games, total_positions, (long)(time(NULL) - t0));
    return 0;
}
